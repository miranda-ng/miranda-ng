/*

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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

struct CSavedTab
{
	CSavedTab(const char *szModule, const wchar_t *wszId) :
		m_szModule(mir_strdup(szModule)),
		m_id(mir_wstrdup(wszId))
	{}

	ptrW m_id;
	ptrA m_szModule;
};

static OBJLIST<CSavedTab> arSavedTabs(1);

void TB_SaveSession(SESSION_INFO *si)
{
	if (si)
		arSavedTabs.insert(new CSavedTab(si->pszModule, si->ptszID));
}

CTabbedWindow *pDialog = nullptr;
int g_iMessageIconIndex;

/////////////////////////////////////////////////////////////////////////////////////////

CTabbedWindow* GetContainer()
{
	if (g_Settings.bTabsEnable) {
		if (pDialog == nullptr) {
			pDialog = new CTabbedWindow();
			pDialog->Show();
		}
		return pDialog;
	}

	return new CTabbedWindow();
}

/////////////////////////////////////////////////////////////////////////////////////////

CTabbedWindow::CTabbedWindow() :
	CDlgBase(g_hInst, IDD_CONTAINER),
	m_tab(this, IDC_TAB),
	m_pEmbed(nullptr)
{
	iX = iY = iWidth = iHeight = m_windowWasCascaded = 0;
}

void CTabbedWindow::OnInitDialog()
{
	SetWindowLongPtr(m_tab.GetHwnd(), GWLP_USERDATA, LPARAM(this));
	mir_subclassWindow(m_tab.GetHwnd(), &CSrmmWindow::TabSubclassProc);

	m_hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, nullptr, WS_CHILD | WS_VISIBLE | SBT_TOOLTIPS | SBARS_SIZEGRIP, 0, 0, 0, 0, m_hwnd, nullptr, g_hInst, nullptr);
	SendMessage(m_hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);

	SetWindowPosition();

	if (!g_Settings.bTabsEnable) {
		m_tab.Hide();
		return;
	}

	LONG_PTR mask = GetWindowLongPtr(m_tab.GetHwnd(), GWL_STYLE);
	if (g_Settings.bTabsAtBottom)
		mask |= TCS_BOTTOM;
	else
		mask &= ~TCS_BOTTOM;
	SetWindowLongPtr(m_tab.GetHwnd(), GWL_STYLE, mask);

	TabCtrl_SetMinTabWidth(m_tab.GetHwnd(), 80);
	TabCtrl_SetImageList(m_tab.GetHwnd(), hIconsList);

	// restore previous tabs
	if (g_Settings.bTabRestore) {
		for (int i = 0; i < arSavedTabs.getCount(); i++) {
			CSavedTab &p = arSavedTabs[i];

			SESSION_INFO *si = pci->SM_FindSession(p.m_id, p.m_szModule);
			if (si)
				AddPage(si);
		}
		arSavedTabs.destroy();
	}
}

void CTabbedWindow::OnDestroy()
{
	DestroyWindow(m_hwndStatus); m_hwndStatus = nullptr;

	SaveWindowPosition(true);

	Utils_SaveWindowPosition(m_hwnd, g_dat.bSavePerContact ? ((m_pEmbed == nullptr) ? 0 : m_pEmbed->m_hContact) : 0, CHAT_MODULE, "room");

	if (m_pEmbed == nullptr)
		pDialog = nullptr;
}

int CTabbedWindow::Resizer(UTILRESIZECONTROL *urc)
{
	if (urc->wId == IDC_TAB) {
		RECT rc;
		GetWindowRect(m_hwndStatus, &rc);
		urc->rcItem.top = 1;
		urc->rcItem.bottom = urc->dlgNewSize.cy - (rc.bottom - rc.top) - 1;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_CUSTOM;
	}
	
	return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM; // status bar
}

/////////////////////////////////////////////////////////////////////////////////////////

CTabbedWindow* CTabbedWindow::AddPage(MCONTACT hContact, wchar_t *pwszText, int iNoActivate)
{
	CSrmmWindow *pDlg = new CSrmmWindow(this, hContact);
	pDlg->m_wszInitialText = pwszText;
	if (iNoActivate != -1)
		pDlg->m_bNoActivate = iNoActivate != 0;

	if (g_Settings.bTabsEnable) {
		m_tab.AddPage(pcli->pfnGetContactDisplayName(hContact, 0), nullptr, pDlg);
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

		CChatRoomDlg *pDlg = new CChatRoomDlg(this, si);
		pDlg->SetParent(m_hwnd);
		m_tab.AddPage(szTemp, nullptr, pDlg);
		FixTabIcons(pDlg);

		m_tab.ActivatePage(m_tab.GetCount() - 1);
	}
	else if (insertAt == -1)
		m_tab.ActivatePage(indexfound);
}

void CTabbedWindow::FixTabIcons(CSrmmBaseDialog *pDlg)
{
	if (pDlg != nullptr) {
		int idx = m_tab.GetDlgIndex(pDlg);
		if (idx == -1)
			return;

		int image = 0;
		if (SESSION_INFO *si = ((CChatRoomDlg*)pDlg)->m_si) {
			if (!(si->wState & GC_EVENT_HIGHLIGHT)) {
				MODULEINFO *mi = pci->MM_FindModule(si->pszModule);
				image = (si->wStatus == ID_STATUS_ONLINE) ? mi->OnlineIconIndex : mi->OfflineIconIndex;
				if (si->wState & STATE_TALK)
					image++;
			}
		}
		else image = g_iMessageIconIndex;

		TCITEM tci = {};
		tci.mask = TCIF_IMAGE;
		TabCtrl_GetItem(m_tab.GetHwnd(), idx, &tci);
		if (tci.iImage != image) {
			tci.iImage = image;
			TabCtrl_SetItem(m_tab.GetHwnd(), idx, &tci);
		}
	}
	else RedrawWindow(m_tab.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
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

void CTabbedWindow::SetMessageHighlight(CChatRoomDlg *pDlg)
{
	if (pDlg != nullptr) {
		if (m_tab.GetDlgIndex(pDlg) == -1)
			return;

		pDlg->m_si->wState |= GC_EVENT_HIGHLIGHT;
		FixTabIcons(pDlg);
		if (g_Settings.bFlashWindowHighlight && GetActiveWindow() != m_hwnd && GetForegroundWindow() != m_hwnd)
			SetTimer(m_hwnd, TIMERID_FLASHWND, 900, nullptr);
	}
	else RedrawWindow(m_tab.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
}

void CTabbedWindow::SetTabHighlight(CChatRoomDlg *pDlg)
{
	if (pDlg != nullptr) {
		if (m_tab.GetDlgIndex(pDlg) == -1)
			return;

		FixTabIcons(pDlg);
		if (g_Settings.bFlashWindow && GetActiveWindow() != m_hwnd && GetForegroundWindow() != m_hwnd)
			SetTimer(m_hwnd, TIMERID_FLASHWND, 900, nullptr);
	}
	else RedrawWindow(m_tab.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
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

	if (Utils_RestoreWindowPosition(m_hwnd, g_dat.bSavePerContact ? m_pEmbed->m_hContact : 0, CHAT_MODULE, "room")) {
		if (g_dat.bSavePerContact) {
			if (Utils_RestoreWindowPosition(m_hwnd, 0, CHAT_MODULE, "room", RWPF_NOMOVE))
				SetWindowPos(m_hwnd, 0, 0, 0, 550, 400, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
		}
		else SetWindowPos(m_hwnd, 0, 0, 0, 550, 400, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
	}

	if (!g_dat.bSavePerContact && g_dat.bCascade)
		Srmm_Broadcast(DM_CASCADENEWWINDOW, (WPARAM)m_hwnd, (LPARAM)&m_windowWasCascaded);
}

void CTabbedWindow::TabClicked()
{
	CChatRoomDlg *pDlg = (CChatRoomDlg*)m_tab.GetActivePage();
	if (pDlg == nullptr)
		return;

	SESSION_INFO *s = pDlg->m_si;
	if (s) {
		if (s->wState & STATE_TALK) {
			s->wState &= ~STATE_TALK;
			db_set_w(s->hContact, s->pszModule, "ApparentMode", 0);
		}

		if (s->wState & GC_EVENT_HIGHLIGHT) {
			s->wState &= ~GC_EVENT_HIGHLIGHT;

			if (pcli->pfnGetEvent(s->hContact, 0))
				pcli->pfnRemoveEvent(s->hContact, GC_FAKE_EVENT);
		}

		FixTabIcons(pDlg);
		if (!s->pDlg) {
			pci->ShowRoom(s);
			SendMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK CSrmmWindow::TabSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CTabbedWindow *pOwner = (CTabbedWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	static BOOL bDragging = FALSE;
	static int iBeginIndex = 0;
	switch (msg) {
	case WM_LBUTTONDOWN:
		{
			TCHITTESTINFO tci = {};
			tci.pt.x = (short)LOWORD(GetMessagePos());
			tci.pt.y = (short)HIWORD(GetMessagePos());
			if (DragDetect(hwnd, tci.pt) && TabCtrl_GetItemCount(hwnd) > 1) {
				tci.flags = TCHT_ONITEM;
				ScreenToClient(hwnd, &tci.pt);
				int i = TabCtrl_HitTest(hwnd, &tci);
				if (i != -1) {
					CSrmmWindow *pDlg = (CSrmmWindow*)pOwner->m_tab.GetNthPage(i);
					if (pDlg) {
						SESSION_INFO *si = pDlg->m_si;
						if (si != nullptr) {
							bool bOnline = db_get_w(si->hContact, si->pszModule, "Status", ID_STATUS_OFFLINE) == ID_STATUS_ONLINE;
							MODULEINFO *mi = pci->MM_FindModule(si->pszModule);
							bDragging = TRUE;
							iBeginIndex = i;
							ImageList_BeginDrag(hIconsList, bOnline ? mi->OnlineIconIndex : mi->OfflineIconIndex, 8, 8);
							ImageList_DragEnter(hwnd, tci.pt.x, tci.pt.y);
							SetCapture(hwnd);
						}
					}
					return TRUE;
				}
			}
			else pOwner->TabClicked();
		}
		break;

	case WM_CAPTURECHANGED:
		bDragging = FALSE;
		ImageList_DragLeave(hwnd);
		ImageList_EndDrag();
		break;

	case WM_MOUSEMOVE:
		if (bDragging) {
			TCHITTESTINFO tci = {};
			tci.pt.x = (short)LOWORD(GetMessagePos());
			tci.pt.y = (short)HIWORD(GetMessagePos());
			ScreenToClient(hwnd, &tci.pt);
			ImageList_DragMove(tci.pt.x, tci.pt.y);
		}
		break;

	case WM_LBUTTONUP:
		if (bDragging && ReleaseCapture()) {
			TCHITTESTINFO tci = {};
			tci.pt.x = (short)LOWORD(GetMessagePos());
			tci.pt.y = (short)HIWORD(GetMessagePos());
			tci.flags = TCHT_ONITEM;
			bDragging = FALSE;
			ImageList_DragLeave(hwnd);
			ImageList_EndDrag();

			ScreenToClient(hwnd, &tci.pt);
			int i = TabCtrl_HitTest(hwnd, &tci);
			if (i != -1 && i != iBeginIndex)
				SendMessage(GetParent(hwnd), GC_DROPPEDTAB, i, iBeginIndex);
		}
		break;

	case WM_LBUTTONDBLCLK:
		{
			TCHITTESTINFO tci = {};
			tci.pt.x = (short)LOWORD(GetMessagePos());
			tci.pt.y = (short)HIWORD(GetMessagePos());
			tci.flags = TCHT_ONITEM;

			ScreenToClient(hwnd, &tci.pt);
			int i = TabCtrl_HitTest(hwnd, &tci);
			if (i != -1 && g_Settings.bTabCloseOnDblClick) {
				CSrmmBaseDialog *pDlg = (CSrmmBaseDialog*)pOwner->m_tab.GetNthPage(i);
				if (pDlg)
					pDlg->CloseTab();
			}
		}
		break;

	case WM_MBUTTONUP:
		TCHITTESTINFO tci = {};
		tci.pt.x = (short)LOWORD(GetMessagePos());
		tci.pt.y = (short)HIWORD(GetMessagePos());
		tci.flags = TCHT_ONITEM;

		ScreenToClient(hwnd, &tci.pt);
		int i = TabCtrl_HitTest(hwnd, &tci);
		if (i != -1)
			SendMessage(GetParent(hwnd), GC_REMOVETAB, 0, (LPARAM)pOwner->m_tab.GetNthPage(i));
		break;
	}

	return mir_callNextSubclass(hwnd, TabSubclassProc, msg, wParam, lParam);
}

INT_PTR CTabbedWindow::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case GC_ADDTAB:
		AddPage((SESSION_INFO*)lParam);
		break;

	case GC_SWITCHNEXTTAB:
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
		break;

	case GC_SWITCHPREVTAB:
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
		break;

	case GC_SWITCHTAB:
		{
			int total = m_tab.GetCount();
			int i = TabCtrl_GetCurSel(m_tab.GetHwnd());
			if (i != -1 && total != -1 && total != 1 && i != lParam && total > lParam) {
				m_tab.ActivatePage(i);
				TabClicked();
			}
		}
		break;

	case GC_REMOVETAB:
		{
			int idx = (lParam) ? m_tab.GetDlgIndex((CDlgBase*)lParam) : TabCtrl_GetCurSel(m_tab.GetHwnd());
			if (idx == -1)
				break;

			m_tab.RemovePage(idx);
			if (m_tab.GetCount() == 0)
				PostMessage(m_hwnd, WM_CLOSE, 0, 0);
			else {
				if (m_tab.GetNthPage(idx) == nullptr)
					idx--;
				m_tab.ActivatePage(idx);
			}
		}
		break;

	case GC_TABCHANGE:
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		// ScrollToBottom();
		break;

	case GC_DROPPEDTAB:
		{
			int begin = (int)lParam;
			int end = (int)wParam;
			if (begin == end)
				break;

			m_tab.SwapPages(begin, end);

			CChatRoomDlg *pDlg = (CChatRoomDlg*)m_tab.GetNthPage(end);
			if (pDlg) {
				FixTabIcons(pDlg);
				m_tab.ActivatePage(end);
			}

			// fix the "fixed" positions
			int tabCount = m_tab.GetCount();
			for (int i = 0; i < tabCount; i++) {
				pDlg = (CChatRoomDlg*)m_tab.GetNthPage(i);
				if (pDlg == nullptr)
					continue;

				SESSION_INFO *si = pDlg->m_si;
				if (si && si->hContact && db_get_w(si->hContact, si->pszModule, "TabPosition", 0) != 0)
					db_set_w(si->hContact, si->pszModule, "TabPosition", i + 1);
			}
		}
		break;

	case GC_RENAMETAB:
		if (CChatRoomDlg *pDlg = (CChatRoomDlg*)lParam) {
			int idx = m_tab.GetDlgIndex(pDlg);
			if (idx == -1)
				break;

			TCITEM tci;
			tci.mask = TCIF_TEXT;
			tci.pszText = pDlg->m_si->ptszName;
			TabCtrl_SetItem(m_tab.GetHwnd(), idx, &tci);
		}
		break;

	case WM_TIMER:
		if (wParam == TIMERID_FLASHWND)
			FlashWindow(m_hwnd, TRUE);
		break;

	case WM_MOVE:
		SaveWindowPosition(false);
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

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->hwndFrom == m_hwndStatus) {
			if (((LPNMHDR)lParam)->code == NM_CLICK || ((LPNMHDR)lParam)->code == NM_RCLICK) {
				NMMOUSE *nm = (NMMOUSE *)lParam;
				RECT rc;
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

				SESSION_INFO *si = ((CChatRoomDlg*)m_tab.GetNthPage(i))->m_si;

				ClientToScreen(GetDlgItem(m_hwnd, IDC_TAB), &tci.pt);
				HMENU hSubMenu = GetSubMenu(g_hMenu, 1);
				TranslateMenu(hSubMenu);

				if (si) {
					WORD w = db_get_w(si->hContact, si->pszModule, "TabPosition", 0);
					if (w == 0)
						CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND | MF_UNCHECKED);
					else
						CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND | MF_CHECKED);
				}
				else CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND | MF_UNCHECKED);

				switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, tci.pt.x, tci.pt.y, 0, m_hwnd, nullptr)) {
				case ID_CLOSE:
					SendMessage(m_hwnd, GC_REMOVETAB, 0, (LPARAM)m_tab.GetNthPage(i));
					break;

				case ID_LOCKPOSITION:
					if (si != 0) {
						if (!(GetMenuState(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND)&MF_CHECKED)) {
							if (si->hContact)
								db_set_w(si->hContact, si->pszModule, "TabPosition", (WORD)(i + 1));
						}
						else db_unset(si->hContact, si->pszModule, "TabPosition");
					}
					break;

				case ID_CLOSEOTHER:
					int tabCount = m_tab.GetCount() - 1;
					if (tabCount > 0) {
						for (tabCount; tabCount >= 0; tabCount--) {
							if (tabCount == i)
								continue;

							m_tab.RemovePage(tabCount);
						}
						m_tab.ActivatePage(0);
					}
					break;
				}
			}
		}
		break;
	}

	LRESULT res = CDlgBase::DlgProc(msg, wParam, lParam);
	if (msg == WM_SIZE) {
		SendMessage(m_hwndStatus, WM_SIZE, 0, 0);
		if (m_pEmbed) {
			RECT rc;
			GetClientRect(m_tab.GetHwnd(), &rc);
			MoveWindow(m_pEmbed->GetHwnd(), 0, 0, rc.right - rc.left, rc.bottom - rc.top, FALSE);
		}
		SaveWindowPosition(false);
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

void UninitTabs()
{
	if (pDialog != nullptr) {
		pDialog->Close();
		pDialog = nullptr;
	}

	arSavedTabs.destroy();
}
