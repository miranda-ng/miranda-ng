/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-23 Miranda NG team,
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

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::LoadSettings()
{
	m_clrInputBG = db_get_dw(0, CHAT_MODULE, "ColorMessageBG", GetSysColor(COLOR_WINDOW));
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, nullptr, &m_clrInputFG);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::ShowFilterMenu()
{
	HWND hwnd = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILTER), m_hwnd, FilterWndProc, (LPARAM)this);
	TranslateDialogDefault(hwnd);

	RECT rc;
	GetWindowRect(m_btnFilter.GetHwnd(), &rc);
	SetWindowPos(hwnd, HWND_TOP, rc.left - 85, (IsWindowVisible(m_btnFilter.GetHwnd()) || IsWindowVisible(m_btnBold.GetHwnd())) ? rc.top - 206 : rc.top - 186, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CMsgDialog::UpdateFilterButton()
{
	CSuper::UpdateFilterButton();

	m_btnFilter.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(m_bFilterEnabled ? IDI_FILTER : IDI_FILTER2));
	m_btnNickList.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(m_bNicklistEnabled ? IDI_NICKLIST : IDI_NICKLIST2, FALSE));
}

void CMsgDialog::UpdateNickList()
{
	int i = m_nickList.SendMsg(LB_GETTOPINDEX, 0, 0);
	m_nickList.SendMsg(LB_SETCOUNT, m_si->getUserList().getCount(), 0);
	m_nickList.SendMsg(LB_SETTOPINDEX, i, 0);

	UpdateTitle();
}

void CMsgDialog::UpdateOptions()
{
	UpdateFilterButton();

	HICON hIcon = ImageList_GetIcon(Clist_GetImageList(), GetImageId(), ILD_TRANSPARENT);
	SendMessage(m_pOwner->m_hwndStatus, SB_SETICON, 0, (LPARAM)hIcon);
	DestroyIcon(hIcon);

	Window_SetIcon_IcoLib(m_pOwner->GetHwnd(), g_plugin.getIconHandle(IDI_CHANMGR));

	m_pLog->UpdateOptions();

	// nicklist
	int ih = Chat_GetTextPixelSize(L"AQGglo", g_Settings.UserListFont, FALSE);
	int ih2 = Chat_GetTextPixelSize(L"AQGglo", g_Settings.UserListHeadingsFont, FALSE);
	int height = db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12);
	int font = ih > ih2 ? ih : ih2;

	// make sure we have space for icon!
	if (g_Settings.bShowContactStatus)
		font = font > 16 ? font : 16;

	m_nickList.SendMsg(LB_SETITEMHEIGHT, 0, height > font ? height : font);
	InvalidateRect(m_nickList.GetHwnd(), nullptr, TRUE);

	CSuper::UpdateOptions();
}

void CMsgDialog::UpdateStatusBar()
{
	wchar_t *ptszDispName = m_si->pMI->ptszModDispName;
	int x = 12;
	x += Chat_GetTextPixelSize(ptszDispName, (HFONT)SendMessage(m_pOwner->m_hwndStatus, WM_GETFONT, 0, 0), TRUE);
	x += GetSystemMetrics(SM_CXSMICON);
	int iStatusbarParts[2] = { x, -1 };
	SendMessage(m_pOwner->m_hwndStatus, SB_SETPARTS, 2, (LPARAM)&iStatusbarParts);

	HICON hIcon = ImageList_GetIcon(Clist_GetImageList(), GetImageId(), ILD_TRANSPARENT);
	SendMessage(m_pOwner->m_hwndStatus, SB_SETICON, 0, (LPARAM)hIcon);
	DestroyIcon(hIcon);

	SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)ptszDispName);
	SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 1, (LPARAM)(m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L""));
	SendMessage(m_pOwner->m_hwndStatus, SB_SETTIPTEXT, 1, (LPARAM)(m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L""));
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CMsgDialog::FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int iFlags;
	static CMsgDialog *pDlg = nullptr;

	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CMsgDialog*)lParam;

		iFlags = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "FilterFlags");
		CheckDlgButton(hwndDlg, IDC_1, iFlags & GC_EVENT_ACTION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_2, iFlags & GC_EVENT_MESSAGE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_3, iFlags & GC_EVENT_NICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_4, iFlags & GC_EVENT_JOIN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_5, iFlags & GC_EVENT_PART ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_6, iFlags & GC_EVENT_TOPIC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_7, iFlags & GC_EVENT_ADDSTATUS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_8, iFlags & GC_EVENT_INFORMATION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_9, iFlags & GC_EVENT_QUIT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_10, iFlags & GC_EVENT_KICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_11, iFlags & GC_EVENT_NOTICE ? BST_CHECKED : BST_UNCHECKED);
		break;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, RGB(60, 60, 150));
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			iFlags = 0;

			if (IsDlgButtonChecked(hwndDlg, IDC_1) == BST_CHECKED)
				iFlags |= GC_EVENT_ACTION;
			if (IsDlgButtonChecked(hwndDlg, IDC_2) == BST_CHECKED)
				iFlags |= GC_EVENT_MESSAGE;
			if (IsDlgButtonChecked(hwndDlg, IDC_3) == BST_CHECKED)
				iFlags |= GC_EVENT_NICK;
			if (IsDlgButtonChecked(hwndDlg, IDC_4) == BST_CHECKED)
				iFlags |= GC_EVENT_JOIN;
			if (IsDlgButtonChecked(hwndDlg, IDC_5) == BST_CHECKED)
				iFlags |= GC_EVENT_PART;
			if (IsDlgButtonChecked(hwndDlg, IDC_6) == BST_CHECKED)
				iFlags |= GC_EVENT_TOPIC;
			if (IsDlgButtonChecked(hwndDlg, IDC_7) == BST_CHECKED)
				iFlags |= GC_EVENT_ADDSTATUS;
			if (IsDlgButtonChecked(hwndDlg, IDC_8) == BST_CHECKED)
				iFlags |= GC_EVENT_INFORMATION;
			if (IsDlgButtonChecked(hwndDlg, IDC_9) == BST_CHECKED)
				iFlags |= GC_EVENT_QUIT;
			if (IsDlgButtonChecked(hwndDlg, IDC_10) == BST_CHECKED)
				iFlags |= GC_EVENT_KICK;
			if (IsDlgButtonChecked(hwndDlg, IDC_11) == BST_CHECKED)
				iFlags |= GC_EVENT_NOTICE;

			if (iFlags & GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			db_set_dw(pDlg->m_hContact, CHAT_MODULE, "FilterFlags", iFlags);
			db_set_dw(pDlg->m_hContact, CHAT_MODULE, "FilterMask", 0xFFFF);

			Chat_SetFilters(pDlg->getChat());
			pDlg->RedrawLog();
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	}

	return FALSE;
}
