/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#define WM_MODULE_UNLOAD (WM_USER + 1)

static int UIEmulateBtnClick(HWND hwndDlg, UINT idcButton)
{
	if (IsWindowEnabled(GetDlgItem(hwndDlg, idcButton)))
		PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(idcButton, BN_CLICKED), (LPARAM)GetDlgItem(hwndDlg, idcButton));
	return 0;
}

struct CCtrlMemo : public CCtrlRichEdit
{
	CCtrlMemo(CDlgBase *dlg, int ctrlId) :
		CCtrlRichEdit(dlg, ctrlId)
	{}

	BOOL OnNotify(int, NMHDR *pnmhdr) override
	{
		ENLINK *param = (ENLINK *)pnmhdr;
		if (pnmhdr->code == EN_LINK && param->msg == WM_LBUTTONUP) {
			CHARRANGE sel;
			SendMessage(param->nmhdr.hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin == sel.cpMax) { // allow link selection
				TEXTRANGE tr;
				tr.chrg = param->chrg;
				tr.lpstrText = (wchar_t *)mir_alloc(sizeof(wchar_t) * (tr.chrg.cpMax - tr.chrg.cpMin + 2));
				SendMessage(param->nmhdr.hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

				Utils_OpenUrlW(tr.lpstrText);
				mir_free(tr.lpstrText);
			}
			return TRUE;
		}
		return FALSE;
	}
};

class CEnterStringDlg : public CDlgBase
{
	int idcControl = 0;
	HANDLE m_hEvent = nullptr;
	ENTER_STRING m_param;

	void ComboLoadRecentStrings()
	{
		for (int i = 0; i < m_param.recentCount; i++) {
			char setting[MAXMODULELABELLENGTH];
			mir_snprintf(setting, "%s%d", m_param.szDataPrefix, i);
			ptrW tszRecent(db_get_wsa(0, m_param.szModuleName, setting));
			if (tszRecent != nullptr)
				combo.AddString(tszRecent);
		}

		if (!combo.GetCount())
			combo.AddString(L"");
	}

	void ComboAddRecentString()
	{
		wchar_t *string = m_param.ptszResult;
		if (!string || !*string)
			return;

		if (combo.FindString(string) != -1)
			return;

		int id;
		combo.AddString(string);
		if ((id = combo.FindString(L"")) != CB_ERR)
			combo.DeleteString(id);

		id = db_get_b(0, m_param.szModuleName, m_param.szDataPrefix, 0);
		char setting[MAXMODULELABELLENGTH];
		mir_snprintf(setting, "%s%d", m_param.szDataPrefix, id);
		db_set_ws(0, m_param.szModuleName, setting, string);
		db_set_b(0, m_param.szModuleName, m_param.szDataPrefix, (id + 1) % idcControl);
	}

	LRESULT onModuleUnload(UINT, WPARAM wParam, LPARAM)
	{
		auto *pPlugin = (HPLUGIN)wParam;
		if (!mir_strcmp(m_param.szModuleName, pPlugin->getModule()))
			EndModal(0);
		return 0;
	}

	UI_MESSAGE_MAP(CEnterStringDlg, CDlgBase);
		UI_MESSAGE(WM_MODULE_UNLOAD, onModuleUnload);
	UI_MESSAGE_MAP_END();

	CTimer m_timer1, m_timer2;
	CCtrlEdit edit1, edit2;
	CCtrlMemo memo;
	CCtrlCombo combo;

public:
	CEnterStringDlg(const ENTER_STRING &param) :
		CDlgBase(g_plugin, IDD_ENTER_STRING),
		m_param(param),
		memo(this, IDC_TXT_RICHEDIT),
		edit1(this, IDC_TXT_SIMPLE),
		edit2(this, IDC_TXT_MULTILINE),
		combo(this, IDC_TXT_COMBO),
		m_timer1(this, 1001),
		m_timer2(this, 1002)
	{
		m_timer1.OnEvent = Callback(this, &CEnterStringDlg::onTimer1);
		m_timer2.OnEvent = Callback(this, &CEnterStringDlg::onTimer2);

		memo.OnChange = edit1.OnChange = edit2.OnChange = combo.OnChange = Callback(this, &CEnterStringDlg::onChange_Field);
	}

	bool OnInitDialog() override
	{
		Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_OTHER_RENAME);
		SetCaption(m_param.caption);

		RECT rc;
		GetWindowRect(m_hwnd, &rc);

		if (m_param.szModuleName)
			m_hEvent = HookEventMessage(ME_SYSTEM_MODULEUNLOAD, m_hwnd, WM_MODULE_UNLOAD);

		switch (m_param.type) {
		case ESF_PASSWORD:
			idcControl = IDC_TXT_PASSWORD;
			SetMinSize(rc.right - rc.left, rc.bottom - rc.top);
			break;

		case ESF_MULTILINE:
			idcControl = IDC_TXT_MULTILINE;
			rc.bottom += (rc.bottom - rc.top) * 2;
			SetWindowPos(m_hwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION);
			break;

		case ESF_COMBO:
			idcControl = IDC_TXT_COMBO;
			SetMinSize(rc.right - rc.left, rc.bottom - rc.top);
			if (m_param.szDataPrefix && m_param.recentCount)
				ComboLoadRecentStrings();
			break;

		case ESF_RICHEDIT:
			idcControl = IDC_TXT_RICHEDIT;
			SendDlgItemMessage(m_hwnd, IDC_TXT_RICHEDIT, EM_AUTOURLDETECT, TRUE, 0);
			SendDlgItemMessage(m_hwnd, IDC_TXT_RICHEDIT, EM_SETEVENTMASK, 0, ENM_LINK);
			rc.bottom += (rc.bottom - rc.top) * 2;
			SetWindowPos(m_hwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION);
			break;

		default: // single string edit field
			idcControl = IDC_TXT_SIMPLE;
			SetMinSize(rc.right - rc.left, rc.bottom - rc.top);
		}

		ShowWindow(GetDlgItem(m_hwnd, idcControl), SW_SHOW);
		if (m_param.ptszInitVal)
			SetDlgItemText(m_hwnd, idcControl, m_param.ptszInitVal);

		if (m_param.szDataPrefix)
			Utils_RestoreWindowPosition(m_hwnd, 0, m_param.szModuleName, m_param.szDataPrefix);

		SetTimer(m_hwnd, 1000, 50, nullptr);

		if (m_param.timeout > 0) {
			m_timer2.Start(1000);
			
			wchar_t buf[128];
			mir_snwprintf(buf, TranslateT("OK (%d)"), m_param.timeout);
			SetDlgItemText(m_hwnd, IDOK, buf);
		}

		return true;
	}

	bool OnApply() override
	{
		HWND hWnd = GetDlgItem(m_hwnd, idcControl);
		int len = GetWindowTextLength(hWnd)+1;
		m_param.ptszResult = (LPTSTR)mir_alloc(sizeof(wchar_t)*len);
		GetWindowText(hWnd, m_param.ptszResult, len);

		if ((m_param.type == ESF_COMBO) && m_param.szDataPrefix && m_param.recentCount)
			ComboAddRecentString();
		return true;
	}

	void OnDestroy() override
	{
		if (m_param.szDataPrefix)
			Utils_SaveWindowPosition(m_hwnd, 0, m_param.szModuleName, m_param.szDataPrefix);

		Window_FreeIcon_IcoLib(m_hwnd);
		UnhookEvent(m_hEvent);
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_TXT_MULTILINE:
		case IDC_TXT_COMBO:
		case IDC_TXT_RICHEDIT:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

		case IDOK:
		case IDCANCEL:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	void onTimer1(CTimer *)
	{
		m_timer1.Stop();
		EnableWindow(GetParent(m_hwnd), TRUE);
	}

	void onTimer2(CTimer *)
	{
		wchar_t buf[128];
		mir_snwprintf(buf, TranslateT("OK (%d)"), --m_param.timeout);
		SetDlgItemText(m_hwnd, IDOK, buf);

		if (m_param.timeout < 0) {
			m_timer1.Stop();
			UIEmulateBtnClick(m_hwnd, IDOK);
		}
	}

	void onChange_Field(CCtrlData*)
	{
		SetDlgItemText(m_hwnd, IDOK, TranslateT("OK"));
		m_timer1.Stop();
	}

	wchar_t* GetResult() const
	{
		return m_param.ptszResult;
	}
};

MIR_APP_DLL(bool) EnterString(ENTER_STRING *pForm)
{
	if (pForm == nullptr)
		return false;

	CEnterStringDlg dlg(*pForm);
	dlg.SetParent(GetForegroundWindow());
	if (!dlg.DoModal())
		return false;

	pForm->ptszResult = dlg.GetResult();
	return true;
}
