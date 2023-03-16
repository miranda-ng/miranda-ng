/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HICON hIcon;
HICON hMarked, hNotMarked;

HWND hComboBox = nullptr;
HWND hComboBoxEdit = nullptr;

HWND hOpClistControl = nullptr;

CSession *pSession = nullptr;

BOOL bChecked = FALSE;

static LRESULT CALLBACK ComboBoxSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ERASEBKGND:
		return TRUE;

	case EM_SETSEL:
		if (!hOpClistControl)
			return HideCaret(hwnd);
		break;

	case WM_GETDLGCODE:
		if (!hOpClistControl)
			return DLGC_WANTARROWS;
		break;

	case WM_SETCURSOR:
		if (!hOpClistControl) {
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			return TRUE;
		}
		break;

	case WM_LBUTTONDOWN:
		if (hOpClistControl)
			break;
		HideCaret(hwnd);

	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
		SendMessage(hComboBox, CB_SHOWDROPDOWN, 1, 0);
		return TRUE;

	case WM_NCLBUTTONDBLCLK:
	case WM_NCLBUTTONDOWN:
		if (!bChecked) {
			pSession->bIsFavorite = true;
			hIcon = hMarked;
			bChecked = TRUE;
			RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);
		}
		else {
			pSession->bIsFavorite = false;
			hIcon = hNotMarked;
			bChecked = FALSE;
			RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);
		}
		pSession->save();
		break;

	case WM_MOUSEMOVE:
		if (hOpClistControl)
			break;

	case WM_NCMOUSEMOVE:
		return TRUE;

	case WM_NCPAINT:
		RECT rc;
		GetClientRect(hwnd, &rc);
		rc.left = rc.right;
		rc.right = rc.left + 16;
		{
			HDC hdc = GetDC(hwnd);
			FillRect(hdc, &rc, (HBRUSH)GetSysColor(COLOR_WINDOW));
			DrawIconEx(hdc, rc.left, 0, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
			ReleaseDC(hwnd, hdc);
		}
		break;

	case WM_NCCALCSIZE:
		{
			NCCALCSIZE_PARAMS *ncsParam = (NCCALCSIZE_PARAMS*)lParam;
			ncsParam->rgrc[0].right -= 16;
		}
		break;

	case WM_NCHITTEST:
		LRESULT lr = mir_callNextSubclass(hwnd, ComboBoxSubclassProc, msg, wParam, lParam);
		if (lr == HTNOWHERE)
			lr = HTOBJECT;
		return lr;
	}
	return mir_callNextSubclass(hwnd, ComboBoxSubclassProc, msg, wParam, lParam);
}

class COptionsDlg : public CDlgBase
{
	void BuildList()
	{
		for (auto &cc : Contacts()) {
			HANDLE hContact = m_clist.FindContact(cc);

			bool bSet = false;
			for (int i = 0; session_list_t[i] > 0; i++)
				if (session_list_t[i] == cc) {
					bSet = true;
					break;
				}

			m_clist.SetCheck(hContact, bSet);
		}
	}

	// returns number of contacts in a session
	int LoadSessionContacts()
	{
		memset(session_list_t, 0, sizeof(session_list_t));
		m_opclist.ResetContent();

		int i = 0;
		for (auto &cc : pSession->contacts)
			m_opclist.AddString(Clist_GetContactDisplayName(session_list_t[i++] = cc));

		return i;
	}

	CCtrlClc m_clist;
	CCtrlCombo m_list;
	CCtrlListBox m_opclist;

	CCtrlEdit edtDelay;
	CCtrlSpin spinCount;
	CCtrlCheck chkExclHidden, chkWarnOnHidden, chkOtherWarnings, chkCrashRecovery;
	CCtrlCheck chkStartDialog, chkLoadLast, chkLast, chkNothing;
	CCtrlCheck chkExitAsk, chkExitSave, chkExitNothing;
	CCtrlButton btnSave, btnEdit, btnDel;

	MCONTACT session_list_t[255];
	bool bSesssionNameChanged = false, bClistChanged = false;

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPTIONS),
		m_list(this, IDC_LIST),
		m_clist(this, IDC_EMCLIST),
		m_opclist(this, IDC_OPCLIST),
		btnDel(this, IDC_DEL),
		btnEdit(this, IDC_EDIT),
		btnSave(this, IDC_SAVE),
		edtDelay(this, IDC_STARTDELAY),
		spinCount(this, IDC_SPIN1, 10, 1),
		chkLast(this, IDC_CHECKLAST),
		chkExitAsk(this, IDC_REXASK),
		chkNothing(this, IDC_RNOTHING),
		chkExitSave(this, IDC_REXSAVE),
		chkLoadLast(this, IDC_RLOADLAST),
		chkExitNothing(this, IDC_REXDSAVE),
		chkStartDialog(this, IDC_STARTDIALOG),
		chkExclHidden(this, IDC_EXCLHIDDEN),
		chkWarnOnHidden(this, IDC_LASTHIDDENWARN),
		chkOtherWarnings(this, IDC_WARNINGS),
		chkCrashRecovery(this, IDC_CRASHRECOVERY)
	{
		CreateLink(edtDelay, g_plugin.iStartupDelay);
		CreateLink(spinCount, g_plugin.iTrackCount);

		CreateLink(chkExclHidden, g_plugin.bExclHidden);
		CreateLink(chkWarnOnHidden, g_plugin.bWarnOnHidden);
		CreateLink(chkOtherWarnings, g_plugin.bOtherWarnings);
		CreateLink(chkCrashRecovery, g_plugin.bCrashRecovery);

		btnDel.OnClick = Callback(this, &COptionsDlg::onClick_Del);
		btnEdit.OnClick = Callback(this, &COptionsDlg::onClick_Edit);
		btnSave.OnClick = Callback(this, &COptionsDlg::onClick_Save);

		chkNothing.OnChange = Callback(this, &COptionsDlg::onChange_Nothing);
		chkLoadLast.OnChange = Callback(this, &COptionsDlg::onChange_LoadLast);
		chkStartDialog.OnChange = Callback(this, &COptionsDlg::onChange_StartDialog);

		chkExitAsk.OnChange = Callback(this, &COptionsDlg::onChange_ExAsk);
		chkExitSave.OnChange = Callback(this, &COptionsDlg::onChange_ExSave);
		chkExitNothing.OnChange = Callback(this, &COptionsDlg::onChange_ExNothing);

		m_list.OnChange = Callback(this, &COptionsDlg::onEditChange_List);
		m_list.OnSelChanged = Callback(this, &COptionsDlg::onSelChange_List);

		m_clist.OnCheckChanged = Callback(this, &COptionsDlg::onCheckChanged_Clist);

		memset(session_list_t, 0, sizeof(session_list_t));
	}

	bool OnInitDialog() override
	{
		COMBOBOXINFO cbi = { 0 };
		cbi.cbSize = sizeof(cbi);

		pSession = nullptr;
		hMarked = g_plugin.getIcon(IDD_SESSION_CHECKED);
		hNotMarked = g_plugin.getIcon(IDD_SESSION_UNCHECKED);

		SetWindowLongPtr(m_clist.GetHwnd(), GWL_STYLE,
				GetWindowLongPtr(m_clist.GetHwnd(), GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
		m_clist.SetExStyle(CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT);
		m_clist.AutoRebuild();

		m_opclist.ResetContent();

		int startupmode = g_plugin.getByte("StartupMode", 3);
		if (startupmode == 1)
			chkStartDialog.SetState(true);
		else if (startupmode == 3)
			chkLast.SetState(true);
		else if (startupmode == 2)
			chkLoadLast.SetState(true);
		else if (startupmode == 0)
			chkNothing.SetState(true);

		int exitmode = g_plugin.getByte("ShutdownMode", 2);
		if (exitmode == 0)
			chkExitNothing.SetState(true);
		else if (exitmode == 1)
			chkExitAsk.SetState(true);
		else if (exitmode == 2)
			chkExitSave.SetState(true);

		LoadSessionToCombobox(m_list, true);
		if (m_list.GetCount()) {
			btnEdit.Enable();
			m_list.SetCurSel(0);
			pSession = (CSession *)m_list.GetItemData(0);
			if (!LoadSessionContacts())
				btnDel.Disable();
		}
		else {
			m_list.Disable();
			btnDel.Disable();
		}

		GetComboBoxInfo(m_list.GetHwnd(), &cbi);
		mir_subclassWindow(cbi.hwndItem, ComboBoxSubclassProc);

		hComboBoxEdit = cbi.hwndItem;
		hComboBox = cbi.hwndCombo;

		SetWindowPos(hComboBoxEdit, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		return true;
	}

	bool OnApply() override
	{
		int iDelay = GetDlgItemInt(m_hwnd, IDC_STARTDELAY, nullptr, FALSE);
		g_plugin.setWord("StartupModeDelay", (uint16_t)iDelay);

		if (chkExitSave.IsChecked())
			g_plugin.setByte("ShutdownMode", 2);
		else if (IsDlgButtonChecked(m_hwnd, IDC_REXDSAVE))
			g_plugin.setByte("ShutdownMode", 0);
		else if (chkExitAsk.IsChecked())
			g_plugin.setByte("ShutdownMode", 1);

		if (chkStartDialog.IsChecked())
			g_plugin.setByte("StartupMode", 1);
		else if (chkLast.IsChecked())
			g_plugin.setByte("StartupMode", 3);
		else if (chkLoadLast.IsChecked())
			g_plugin.setByte("StartupMode", 2);
		else if (chkNothing.IsChecked())
			g_plugin.setByte("StartupMode", 0);
		return true;
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case WM_CTLCOLORLISTBOX:
			switch (GetDlgCtrlID((HWND)lParam)) {
			case IDC_OPCLIST:
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			}
			break;

		case WM_NOTIFY:
			if (LOWORD(wParam) == IDC_LIST && !hOpClistControl) {
				if (HIWORD(wParam) == CBN_DROPDOWN) {
					SendMessage(hComboBoxEdit, EM_SETSEL, 0, 0);
					SendMessage(hComboBoxEdit, EM_SCROLLCARET, 0, 0);
					SendMessage(hComboBoxEdit, WM_KILLFOCUS, 0, 0);
				}
				else SendMessage(hComboBoxEdit, EM_SCROLLCARET, 0, 0);

				HideCaret(hComboBoxEdit);
			}
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onCheckChanged_Clist(CCtrlClc::TEventInfo*)
	{
		btnSave.Enable();
		bClistChanged = true;
	}

	void onEditChange_List(CCtrlCombo *)
	{
		btnSave.Enable();
		bSesssionNameChanged = true;
	}

	void onSelChange_List(CCtrlCombo *)
	{
		int index = m_list.GetCurSel();
		if (index == CB_ERR)
			return;

		pSession = (CSession*)m_list.GetItemData(index);
		m_opclist.ResetContent();
		if (pSession->bIsFavorite) {
			hIcon = hMarked;
			bChecked = TRUE;
			RedrawWindow(hComboBoxEdit, nullptr, nullptr, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW | RDW_FRAME);
		}
		else {
			hIcon = hNotMarked;
			bChecked = FALSE;
			RedrawWindow(hComboBoxEdit, nullptr, nullptr, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW | RDW_FRAME);
		}
		LoadSessionContacts();
		if (!hOpClistControl)
			btnDel.Enable();
		else {
			BuildList();
			btnSave.Disable();
		}
	}

	void onClick_Edit(CCtrlButton *)
	{
		if (!hOpClistControl) {
			m_opclist.Hide();
			m_clist.Show();
			btnDel.Disable();
			btnEdit.SetText(TranslateT("View"));

			hOpClistControl = m_clist.GetHwnd();
			BuildList();
		}
		else {
			if (btnSave.Enabled()) {
				if (IDYES == MessageBoxW(m_hwnd, TranslateT("Do you want to save changes?"), TranslateT("Sessions"), MB_YESNO | MB_ICONQUESTION))
					onClick_Save(0);
			}

			m_clist.Hide();
			m_opclist.Show();
			btnDel.Enable();
			btnSave.Disable();
			btnEdit.SetText(TranslateT("Edit"));
			hOpClistControl = nullptr;
		}
	}

	void onClick_Save(CCtrlButton *)
	{
		if (bClistChanged) {
			pSession->contacts.clear();

			for (auto &hContact : Contacts()) {
				uint8_t res = m_clist.GetCheck(m_clist.FindContact(hContact));
				if (res)
					pSession->contacts.push_back(hContact);
			}
			
			LoadSessionContacts();
		}

		if (bSesssionNameChanged) {
			if (GetWindowTextLength(hComboBoxEdit)) {
				wchar_t szUserSessionName[MAX_PATH] = { '\0' };
				GetWindowText(hComboBoxEdit, szUserSessionName, _countof(szUserSessionName));
				pSession->wszName = szUserSessionName;

				m_list.ResetContent();
				LoadSessionToCombobox(m_list, true);
			}
			bSesssionNameChanged = false;
		}
		pSession->save();

		btnSave.Disable();
	}

	void onClick_Del(CCtrlButton *)
	{
		pSession->remove();

		m_opclist.ResetContent();
		m_list.ResetContent();

		LoadSessionToCombobox(m_list, true);

		if (m_list.GetCount()) {
			btnEdit.Enable();
			m_list.SetCurSel(0);
			if (!LoadSessionContacts())
				btnDel.Disable();
		}
		else {
			pSession = nullptr;
			m_list.Disable();
			btnEdit.Disable();
			btnDel.Disable();
		}
	}

	void onChange_StartDialog(CCtrlCheck *pCheck)
	{
		if (!pCheck->IsChecked()) return;

		EnableWindow(GetDlgItem(m_hwnd, IDC_STARTDELAY), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATICOP), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATICMS), TRUE);
	}

	void onChange_LoadLast(CCtrlCheck *pCheck)
	{
		if (!pCheck->IsChecked()) return;

		EnableWindow(GetDlgItem(m_hwnd, IDC_STARTDELAY), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATICOP), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATICMS), TRUE);
	}

	void onChange_Nothing(CCtrlCheck *pCheck)
	{
		if (!pCheck->IsChecked()) return;

		EnableWindow(GetDlgItem(m_hwnd, IDC_STARTDELAY), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATICOP), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATICMS), FALSE);
	}

	void onChange_ExSave(CCtrlCheck *pCheck)
	{
		if (!pCheck->IsChecked()) return;

		EnableWindow(GetDlgItem(m_hwnd, IDC_EXSTATIC1), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EXSTATIC2), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TRACK), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SPIN1), TRUE);
	}

	void onChange_ExNothing(CCtrlCheck *pCheck)
	{
		if (!pCheck->IsChecked()) return;

		EnableWindow(GetDlgItem(m_hwnd, IDC_EXSTATIC1), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EXSTATIC2), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TRACK), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SPIN1), FALSE);
	}

	void onChange_ExAsk(CCtrlCheck *pCheck)
	{
		if (!pCheck->IsChecked()) return;

		EnableWindow(GetDlgItem(m_hwnd, IDC_EXSTATIC1), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EXSTATIC2), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TRACK), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SPIN1), TRUE);
	}
};

int OptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = MODULENAME;
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = 955000000;
	odp.pDialog = new COptionsDlg();
	g_plugin.addOptions(wparam, &odp);
	return 0;
}
