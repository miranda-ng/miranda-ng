/*

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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
#include "statusicon.h"

CTabbedWindow *g_pTabDialog = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////

CTabbedWindow* GetContainer()
{
	if (g_Settings.bTabsEnable) {
		if (g_pTabDialog == nullptr) {
			g_pTabDialog = new CTabbedWindow();
			g_pTabDialog->Show();
		}
		return g_pTabDialog;
	}

	return new CTabbedWindow();
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK TabSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CTabbedWindow *pOwner = (CTabbedWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	TCHITTESTINFO tci = {};

	static bool bDragging = false;
	static int iBeginIndex = 0;
	switch (msg) {
	case WM_LBUTTONDOWN:
		tci.pt.x = (short)LOWORD(GetMessagePos());
		tci.pt.y = (short)HIWORD(GetMessagePos());
		if (DragDetect(hwnd, tci.pt) && TabCtrl_GetItemCount(hwnd) > 1) {
			tci.flags = TCHT_ONITEM;
			ScreenToClient(hwnd, &tci.pt);
			int idx = TabCtrl_HitTest(hwnd, &tci);
			if (idx != -1) {
				CMsgDialog *pDlg = (CMsgDialog*)pOwner->m_tab.GetNthPage(idx);
				if (pDlg) {
					bDragging = true;
					iBeginIndex = idx;
					ImageList_BeginDrag(Clist_GetImageList(), pDlg->GetImageId(), 8, 8);
					ImageList_DragEnter(hwnd, tci.pt.x, tci.pt.y);
					SetCapture(hwnd);
				}
				return TRUE;
			}
		}
		else pOwner->TabClicked();
		break;

	case WM_CAPTURECHANGED:
		bDragging = false;
		ImageList_DragLeave(hwnd);
		ImageList_EndDrag();
		break;

	case WM_MOUSEMOVE:
		if (bDragging) {
			tci.pt.x = (short)LOWORD(GetMessagePos());
			tci.pt.y = (short)HIWORD(GetMessagePos());
			ScreenToClient(hwnd, &tci.pt);
			ImageList_DragMove(tci.pt.x, tci.pt.y);
		}
		break;

	case WM_LBUTTONUP:
		if (bDragging && ReleaseCapture()) {
			tci.pt.x = (short)LOWORD(GetMessagePos());
			tci.pt.y = (short)HIWORD(GetMessagePos());
			tci.flags = TCHT_ONITEM;
			bDragging = false;
			ImageList_DragLeave(hwnd);
			ImageList_EndDrag();

			ScreenToClient(hwnd, &tci.pt);
			int idx = TabCtrl_HitTest(hwnd, &tci);
			if (idx != -1 && idx != iBeginIndex)
				pOwner->DropTab(idx, iBeginIndex);
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (g_Settings.bTabCloseOnDblClick) {
			tci.pt.x = (short)LOWORD(GetMessagePos());
			tci.pt.y = (short)HIWORD(GetMessagePos());
			ScreenToClient(hwnd, &tci.pt);

			tci.flags = TCHT_ONITEM;
			int idx = TabCtrl_HitTest(hwnd, &tci);
			if (idx != -1) {
				CMsgDialog *pDlg = (CMsgDialog*)pOwner->m_tab.GetNthPage(idx);
				if (pDlg)
					pDlg->CloseTab();
			}
		}
		break;

	case WM_MBUTTONUP:
		tci.pt.x = (short)LOWORD(GetMessagePos());
		tci.pt.y = (short)HIWORD(GetMessagePos());
		tci.flags = TCHT_ONITEM;

		ScreenToClient(hwnd, &tci.pt);
		int idx = TabCtrl_HitTest(hwnd, &tci);
		if (idx != -1) {
			CMsgDialog *pDlg = (CMsgDialog*)pOwner->m_tab.GetNthPage(idx);
			if (pDlg)
				pDlg->CloseTab();
		}
		break;
	}

	return mir_callNextSubclass(hwnd, TabSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

CTabbedWindow::CTabbedWindow() :
	CDlgBase(g_plugin, IDD_CONTAINER),
	m_tab(this, IDC_TAB)
{
	SetMinSize(450, 350);
}

bool CTabbedWindow::OnInitDialog()
{
	SetWindowLongPtr(m_tab.GetHwnd(), GWLP_USERDATA, LPARAM(this));
	mir_subclassWindow(m_tab.GetHwnd(), ::TabSubclassProc);

	m_hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, nullptr, WS_CHILD | WS_VISIBLE | SBT_TOOLTIPS | SBARS_SIZEGRIP, 0, 0, 0, 0, m_hwnd, nullptr, g_plugin.getInst(), nullptr);
	SendMessage(m_hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);

	RECT rc;
	GetWindowRect(m_hwndStatus, &rc);
	m_statusHeight = rc.bottom - rc.top;

	SetWindowPosition();

	if (!g_Settings.bTabsEnable) {
		m_tab.Hide();
		return false;
	}

	LONG_PTR mask = GetWindowLongPtr(m_tab.GetHwnd(), GWL_STYLE);
	if (g_Settings.bTabsAtBottom)
		mask |= TCS_BOTTOM;
	else
		mask &= ~TCS_BOTTOM;
	SetWindowLongPtr(m_tab.GetHwnd(), GWL_STYLE, mask);

	TabCtrl_SetMinTabWidth(m_tab.GetHwnd(), 80);
	TabCtrl_SetImageList(m_tab.GetHwnd(), Clist_GetImageList());
	return true;
}

void CTabbedWindow::OnDestroy()
{
	DestroyWindow(m_hwndStatus); m_hwndStatus = nullptr;

	SaveWindowPosition(true);

	Utils_SaveWindowPosition(m_hwnd, g_plugin.bSavePerContact ? ((m_pEmbed == nullptr) ? 0 : m_pEmbed->m_hContact) : 0, CHAT_MODULE, "room");

	if (m_pEmbed == nullptr)
		g_pTabDialog = nullptr;
}

void CTabbedWindow::OnResize()
{
	CDlgBase::OnResize();

	SendMessage(m_hwndStatus, WM_SIZE, 0, 0);

	if (m_pEmbed) {
		RECT rc;
		GetClientRect(m_tab.GetHwnd(), &rc);
		MoveWindow(m_pEmbed->GetHwnd(), 0, 0, rc.right - rc.left, rc.bottom - rc.top, FALSE);
	}

	SaveWindowPosition(false);
}

int CTabbedWindow::Resizer(UTILRESIZECONTROL *urc)
{
	if (urc->wId == IDC_TAB) {
		urc->rcItem.top = 1;
		urc->rcItem.bottom = urc->dlgNewSize.cy - m_statusHeight - 1;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_CUSTOM;
	}
	
	return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM; // status bar
}

/////////////////////////////////////////////////////////////////////////////////////////

CTabbedWindow* CTabbedWindow::AddPage(MCONTACT hContact, wchar_t *pwszText, int iNoActivate)
{
	CMsgDialog *pDlg = new CMsgDialog(this, hContact);
	pDlg->m_wszInitialText = pwszText;
	if (iNoActivate != -1)
		pDlg->m_bNoActivate = iNoActivate != 0;

	if (g_Settings.bTabsEnable) {
		m_tab.AddPage(Clist_GetContactDisplayName(hContact), nullptr, pDlg);
		FixTabIcons(pDlg);

		m_tab.ActivatePage(m_tab.GetCount() - 1);
	}
	else {
		m_pEmbed = pDlg;
		Create();
		pDlg->SetParent(m_hwnd);
		pDlg->Create();
		Show();
	}

	PostMessage(m_hwnd, WM_SIZE, 0, 0);
	return this;
}

void CTabbedWindow::AddPage(SESSION_INFO *si, int insertAt)
{ 
	// does the tab already exist?
	int indexfound = (si->pDlg) ? m_tab.GetDlgIndex(si->pDlg) : -1;
	if (indexfound == -1) { // create a new tab
		wchar_t szTemp[30];
		mir_wstrncpy(szTemp, si->ptszName, 21);
		if (mir_wstrlen(si->ptszName) > 20)
			mir_wstrncpy(szTemp + 20, L"...", 4);

		if (!IsWindowVisible(m_hwnd))
			Show(SW_SHOW);

		CMsgDialog *pDlg = new CMsgDialog(this, si);
		pDlg->SetParent(m_hwnd);
		m_tab.AddPage(szTemp, nullptr, pDlg);
		m_tab.ActivatePage(m_tab.GetCount() - 1);
		FixTabIcons(pDlg);
	}
	else if (insertAt == -1)
		m_tab.ActivatePage(indexfound);
}

CMsgDialog* CTabbedWindow::CurrPage() const
{
	return (m_pEmbed != nullptr) ? m_pEmbed : (CMsgDialog*)m_tab.GetActivePage();
}

void CTabbedWindow::DropTab(int begin, int end)
{
	if (begin == end)
		return;

	m_tab.SwapPages(begin, end);

	CMsgDialog *pDlg = (CMsgDialog *)m_tab.GetNthPage(end);
	if (pDlg) {
		FixTabIcons(pDlg);
		m_tab.ActivatePage(end);
	}

	// fix the "fixed" positions
	int tabCount = m_tab.GetCount();
	for (int i = 0; i < tabCount; i++) {
		pDlg = (CMsgDialog *)m_tab.GetNthPage(i);
		if (pDlg == nullptr)
			continue;

		SESSION_INFO *si = pDlg->m_si;
		if (si && si->hContact && db_get_w(si->hContact, si->pszModule, "TabPosition", 0) != 0)
			db_set_w(si->hContact, si->pszModule, "TabPosition", i + 1);
	}
}

void CTabbedWindow::FixTabIcons(CMsgDialog *pDlg)
{
	if (pDlg == nullptr)
		return;

	int image = pDlg->GetImageId();

	// if tabs are turned off, simply change the window's icon, otherwise set the tab's icon first
	if (m_pEmbed == nullptr) {
		int idx = m_tab.GetDlgIndex(pDlg);
		if (idx == -1)
			return;

		TCITEM tci = {};
		tci.mask = TCIF_IMAGE;
		TabCtrl_GetItem(m_tab.GetHwnd(), idx, &tci);
		if (tci.iImage != image) {
			tci.iImage = image;
			TabCtrl_SetItem(m_tab.GetHwnd(), idx, &tci);
		}
	}

	// set the container's icon only if we're processing the current page
	if (pDlg == CurrPage()) {
		Window_FreeIcon_IcoLib(m_hwnd);
		if (g_plugin.bUseStatusWinIcon)
			Window_SetProtoIcon_IcoLib(m_hwnd, pDlg->m_szProto, pDlg->m_wStatus);
		else if (pDlg->isChat())
			Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_CHANMGR));
		else
			Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_EVENT_MESSAGE);
	}
}

void CTabbedWindow::RemoveTab(CMsgDialog *pDlg)
{
	int idx = m_tab.GetDlgIndex(pDlg);
	if (idx == -1)
		return;

	m_tab.RemovePage(idx);
	if (m_tab.GetCount() == 0)
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	else {
		if (m_tab.GetNthPage(idx) == nullptr)
			idx--;
		m_tab.ActivatePage(idx);
	}
}

void CTabbedWindow::SaveWindowPosition(bool bUpdateSession)
{
	WINDOWPLACEMENT wp = {};
	wp.length = sizeof(wp);
	GetWindowPlacement(m_hwnd, &wp);

	g_Settings.iX = wp.rcNormalPosition.left;
	g_Settings.iY = wp.rcNormalPosition.top;
	g_Settings.iWidth = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	g_Settings.iHeight = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

	if (bUpdateSession) {
		iX = g_Settings.iX;
		iY = g_Settings.iY;
		iWidth = g_Settings.iWidth;
		iHeight = g_Settings.iHeight;
	}
}

void CTabbedWindow::SetMessageHighlight(CMsgDialog *pDlg)
{
	if (m_tab.GetDlgIndex(pDlg) == -1)
		return;

	pDlg->m_si->wState |= GC_EVENT_HIGHLIGHT;
	FixTabIcons(pDlg);
	if (Chat::bFlashWindowHighlight && pDlg != m_tab.GetActivePage())
		pDlg->StartFlash();
}

void CTabbedWindow::SetTabHighlight(CMsgDialog *pDlg)
{
	if (m_tab.GetDlgIndex(pDlg) == -1)
		return;

	FixTabIcons(pDlg);
	if (Chat::bFlashWindow && pDlg != m_tab.GetActivePage())
		pDlg->StartFlash();
}

void CTabbedWindow::SetWindowPosition()
{
	if (m_pEmbed == nullptr) {
		Utils_RestoreWindowPosition(m_hwnd, 0, CHAT_MODULE, "room");
		return;
	}

	if (iX) {
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		GetWindowPlacement(m_hwnd, &wp);

		wp.rcNormalPosition.left = iX;
		wp.rcNormalPosition.top = iY;
		wp.rcNormalPosition.right = wp.rcNormalPosition.left + iWidth;
		wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + iHeight;
		wp.showCmd = SW_HIDE;
		SetWindowPlacement(m_hwnd, &wp);
		return;
	}

	int flag = m_pEmbed->m_bNoActivate ? RWPF_HIDDEN : 0;
	if (Utils_RestoreWindowPosition(m_hwnd, g_plugin.bSavePerContact ? m_pEmbed->m_hContact : 0, CHAT_MODULE, "room", flag)) {
		if (g_plugin.bSavePerContact) {
			if (Utils_RestoreWindowPosition(m_hwnd, 0, CHAT_MODULE, "room", flag | RWPF_NOMOVE))
				SetWindowPos(m_hwnd, nullptr, 0, 0, 550, 400, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
		}
		else SetWindowPos(m_hwnd, nullptr, 0, 0, 550, 400, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
	}

	if (!g_plugin.bSavePerContact && g_plugin.bCascade) {
		RECT rc, rcMax = {};
		CTabbedWindow *pMaxTab = nullptr;

		for (auto &it : g_arDialogs) {
			auto *pTab = it->m_pOwner;
			if (pTab == this)
				continue;

			GetWindowRect(pTab->GetHwnd(), &rc);
			if (rc.left > rcMax.left && rc.top > rcMax.top) {
				pMaxTab = pTab;
				rcMax = rc;
			}
		}

		if (pMaxTab) {
			m_windowWasCascaded = 1;
			int offset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
			SetWindowPos(m_hwnd, nullptr, rcMax.left + offset, rcMax.top + offset, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		}
	}
}

void CTabbedWindow::SwitchNextTab()
{
	int total = m_tab.GetCount();
	int i = TabCtrl_GetCurSel(m_tab.GetHwnd());
	if (i != -1 && total > 1) {
		if (i < total - 1)
			i++;
		else
			i = 0;
		m_tab.ActivatePage(i);
		TabClicked();
	}
}

void CTabbedWindow::SwitchPrevTab()
{
	int total = m_tab.GetCount();
	int i = TabCtrl_GetCurSel(m_tab.GetHwnd());
	if (i != -1 && total >= 1) {
		if (i > 0)
			i--;
		else
			i = total - 1;
		m_tab.ActivatePage(i);
		TabClicked();
	}
}

void CTabbedWindow::SwitchTab(int iNewTab)
{
	int total = m_tab.GetCount();
	int i = TabCtrl_GetCurSel(m_tab.GetHwnd());
	if (i != -1 && total != -1 && total != 1 && i != iNewTab && total > iNewTab) {
		m_tab.ActivatePage(iNewTab);
		TabClicked();
	}
}

void CTabbedWindow::TabClicked()
{
	CMsgDialog *pDlg = (CMsgDialog*)m_tab.GetActivePage();
	if (pDlg == nullptr)
		return;

	SetFocus(pDlg->m_message.GetHwnd());

	SESSION_INFO *si = pDlg->m_si;
	if (si) {
		if (si->wState & STATE_TALK) {
			si->wState &= ~STATE_TALK;
			db_set_w(si->hContact, si->pszModule, "ApparentMode", 0);
		}

		if (si->wState & GC_EVENT_HIGHLIGHT) {
			si->wState &= ~GC_EVENT_HIGHLIGHT;

			if (g_clistApi.pfnGetEvent(si->hContact, 0))
				g_clistApi.pfnRemoveEvent(si->hContact, GC_FAKE_EVENT);
		}

		if (!si->pDlg) {
			g_chatApi.ShowRoom(si);
			SendMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
		}
	}

	FixTabIcons(pDlg);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CTabbedWindow::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	int idx;

	switch (msg) {
	case WM_MOVE:
		SaveWindowPosition(false);
		break;

	case WM_ENTERSIZEMOVE:
		GetClientRect(m_hwnd, &rc);
		oldSizeX = rc.right - rc.left;
		oldSizeY = rc.bottom - rc.top;
		break;

	case WM_EXITSIZEMOVE:
		GetClientRect(m_hwnd, &rc);
		if (!((rc.right - rc.left) == oldSizeX && (rc.bottom - rc.top) == oldSizeY)) {
			CMsgDialog *pDlg = (g_Settings.bTabsEnable) ? (CMsgDialog*)m_tab.GetActivePage() : m_pEmbed;
			if (pDlg != nullptr) {
				pDlg->m_pLog->ScrollToBottom();
				pDlg->Resize();
			}
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->hwndItem == m_hwndStatus) {
				CMsgDialog *pDlg = (g_Settings.bTabsEnable) ? (CMsgDialog*)m_tab.GetActivePage() : m_pEmbed;
				if (pDlg != nullptr)
					DrawStatusIcons(pDlg->m_hContact, dis->hDC, dis->rcItem, 2);
				return TRUE;
			}
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			CMsgDialog *pDlg = (g_Settings.bTabsEnable) ? (CMsgDialog*)m_tab.GetActivePage() : m_pEmbed;
			if (pDlg != nullptr) {
				pDlg->m_btnOk.Click();
				return TRUE;
			}
		}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
			break;

		if (!m_pEmbed) {
			idx = TabCtrl_GetCurSel(m_tab.GetHwnd());
			if (idx != -1)
				m_tab.ActivatePage(idx);
		}
		else m_pEmbed->OnActivate();
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->hwndFrom == m_hwndStatus) {
			if (((LPNMHDR)lParam)->code == NM_CLICK || ((LPNMHDR)lParam)->code == NM_RCLICK) {
				NMMOUSE *nm = (NMMOUSE *)lParam;
				SendMessage(m_hwndStatus, SB_GETRECT, SendMessage(m_hwndStatus, SB_GETPARTS, 0, 0) - 1, (LPARAM)&rc);
				if (nm->pt.x >= rc.left) {
					CMsgDialog *pDlg = (g_Settings.bTabsEnable) ? (CMsgDialog*)m_tab.GetActivePage() : m_pEmbed;
					if (pDlg != nullptr)
						CheckStatusIconClick(pDlg->m_hContact, m_hwndStatus, nm->pt, rc, 2, ((LPNMHDR)lParam)->code == NM_RCLICK ? MBCF_RIGHTBUTTON : 0);
				}
				return TRUE;
			}
		}

		if (((LPNMHDR)lParam)->idFrom == IDC_TAB) {
			switch (((LPNMHDR)lParam)->code) {
			case TCN_SELCHANGE:
				m_tab.ActivatePage(TabCtrl_GetCurSel(m_tab.GetHwnd()));
				break;

			case NM_RCLICK:
				int i = TabCtrl_GetCurSel(((LPNMHDR)lParam)->hwndFrom);
				if (i == -1)
					break;

				TCHITTESTINFO tci = {};
				tci.pt.x = (short)LOWORD(GetMessagePos());
				tci.pt.y = (short)HIWORD(GetMessagePos());
				tci.flags = TCHT_ONITEM;
				ScreenToClient(GetDlgItem(m_hwnd, IDC_TAB), &tci.pt);
				if ((i = TabCtrl_HitTest(((LPNMHDR)lParam)->hwndFrom, &tci)) == -1)
					break;

				CMsgDialog *pDlg = (CMsgDialog*)m_tab.GetNthPage(i);
				SESSION_INFO *si = pDlg->m_si;

				ClientToScreen(GetDlgItem(m_hwnd, IDC_TAB), &tci.pt);
				HMENU hSubMenu = GetSubMenu(g_hMenu, 1);
				TranslateMenu(hSubMenu);

				if (si != nullptr) {
					uint16_t w = db_get_w(si->hContact, si->pszModule, "TabPosition", 0);
					if (w == 0)
						CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND | MF_UNCHECKED);
					else
						CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND | MF_CHECKED);
				}
				else CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND | MF_UNCHECKED);

				switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, tci.pt.x, tci.pt.y, 0, m_hwnd, nullptr)) {
				case ID_CLOSE:
					pDlg->CloseTab();
					break;

				case ID_LOCKPOSITION:
					if (si != nullptr) {
						if (!(GetMenuState(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND)&MF_CHECKED)) {
							if (si->hContact)
								db_set_w(si->hContact, si->pszModule, "TabPosition", (uint16_t)(i + 1));
						}
						else db_unset(si->hContact, si->pszModule, "TabPosition");
					}
					break;

				case ID_CLOSEOTHER:
					int tabCount = m_tab.GetCount();
					if (tabCount > 1) {
						while (tabCount--) {
							if (tabCount == i)
								continue;

							if (pDlg = (CMsgDialog*)m_tab.GetNthPage(tabCount))
								pDlg->CloseTab();
						}
						m_tab.ActivatePage(0);
					}
					break;
				}
			}
		}
		break;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

void UninitTabs()
{
	if (g_pTabDialog != nullptr) {
		g_pTabDialog->Close();
		g_pTabDialog = nullptr;
	}
}
