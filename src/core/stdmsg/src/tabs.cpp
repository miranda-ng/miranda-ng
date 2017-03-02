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

/////////////////////////////////////////////////////////////////////////////////////////

struct CTabbedWindow : public CDlgBase
{
	CCtrlPages m_tab;

	CTabbedWindow() :
		CDlgBase(g_hInst, IDD_CONTAINER),
		m_tab(this, IDC_TAB)
	{}

	void AddPage(SESSION_INFO*, int insertAt = -1);
	void TabClicked();

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual int Resizer(UTILRESIZECONTROL *urc)
	{
		if (urc->wId == IDC_TAB)
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
};

static LRESULT CALLBACK TabSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
					TCITEM tc;
					tc.mask = TCIF_PARAM;
					TabCtrl_GetItem(hwnd, i, &tc);
					SESSION_INFO *s = (SESSION_INFO*)tc.lParam;
					if (s) {
						BOOL bOnline = db_get_w(s->hContact, s->pszModule, "Status", ID_STATUS_OFFLINE) == ID_STATUS_ONLINE ? TRUE : FALSE;
						MODULEINFO *mi = pci->MM_FindModule(s->pszModule);
						bDragging = TRUE;
						iBeginIndex = i;
						ImageList_BeginDrag(hIconsList, bOnline ? mi->OnlineIconIndex : mi->OfflineIconIndex, 8, 8);
						ImageList_DragEnter(hwnd, tci.pt.x, tci.pt.y);
						SetCapture(hwnd);
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
			if (i != -1 && g_Settings.bTabCloseOnDblClick)
				PostMessage(GetParent(hwnd), GC_CLOSEWINDOW, 0, 0);
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

void CTabbedWindow::OnInitDialog()
{
	SetWindowLongPtr(m_tab.GetHwnd(), GWLP_USERDATA, LPARAM(this));
	mir_subclassWindow(m_tab.GetHwnd(), TabSubclassProc);

	if (db_get_b(NULL, CHAT_MODULE, "SavePosition", 0))
		RestoreWindowPosition(m_hwnd, NULL, false);

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
			ShowWindow(m_hwnd, SW_SHOW);

		CChatRoomDlg *pTab = new CChatRoomDlg(si);
		m_tab.AddPage(szTemp, NULL, pTab);
		SendMessage(m_hwnd, GC_FIXTABICONS, 0, (LPARAM)pTab);

		m_tab.ActivatePage(m_tab.GetCount() - 1);
	}
	else if (insertAt == -1)
		m_tab.ActivatePage(indexfound);
}

void CTabbedWindow::TabClicked()
{
	CDlgBase *pDlg = m_tab.GetActivePage();
	if (pDlg == NULL)
		return;

	SESSION_INFO *s = ((CChatRoomDlg*)pDlg)->m_si;
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

		SendMessage(m_hwnd, GC_FIXTABICONS, 0, (LPARAM)pDlg);
		if (!s->hWnd) {
			pci->ShowRoom(s);
			SendMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
		}
	}
}

INT_PTR CTabbedWindow::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case GC_ADDTAB:
		AddPage((SESSION_INFO*)lParam);
		break;

	case GC_FIXTABICONS:
		if (CChatRoomDlg *pDlg = (CChatRoomDlg*)lParam) {
			int idx = m_tab.GetDlgIndex(pDlg);
			if (idx == -1)
				break;

			SESSION_INFO *si = pDlg->m_si;
			int image = 0;
			if (!(si->wState & GC_EVENT_HIGHLIGHT)) {
				MODULEINFO *mi = pci->MM_FindModule(si->pszModule);
				image = (si->wStatus == ID_STATUS_ONLINE) ? mi->OnlineIconIndex : mi->OfflineIconIndex;
				if (si->wState & STATE_TALK)
					image++;
			}

			TCITEM tci = {};
			tci.mask = TCIF_IMAGE;
			TabCtrl_GetItem(m_tab.GetHwnd(), idx, &tci);
			if (tci.iImage != image) {
				tci.iImage = image;
				TabCtrl_SetItem(m_tab.GetHwnd(), idx, &tci);
			}
		}
		else RedrawWindow(m_tab.GetHwnd(), NULL, NULL, RDW_INVALIDATE);
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
				if (m_tab.GetNthPage(idx) == NULL)
					idx--;
				m_tab.ActivatePage(idx);
			}
		}
		break;

	case GC_SETMESSAGEHIGHLIGHT:
		if (CChatRoomDlg *pDlg = (CChatRoomDlg*)lParam) {
			if (m_tab.GetDlgIndex(pDlg) == -1)
				break;

			pDlg->m_si->wState |= GC_EVENT_HIGHLIGHT;
			SendMessage(m_hwnd, GC_FIXTABICONS, 0, (LPARAM)pDlg);
			if (g_Settings.bFlashWindowHighlight && GetActiveWindow() != m_hwnd && GetForegroundWindow() != m_hwnd)
				SetTimer(m_hwnd, TIMERID_FLASHWND, 900, NULL);
		}
		else RedrawWindow(m_tab.GetHwnd(), NULL, NULL, RDW_INVALIDATE);
		break;

	case GC_SETTABHIGHLIGHT:
		if (CChatRoomDlg *pDlg = (CChatRoomDlg*)lParam) {
			if (m_tab.GetDlgIndex(pDlg) == -1)
				break;

			SendMessage(m_hwnd, GC_FIXTABICONS, 0, (LPARAM)pDlg);
			if (g_Settings.bFlashWindow && GetActiveWindow() != m_hwnd && GetForegroundWindow() != m_hwnd)
				SetTimer(m_hwnd, TIMERID_FLASHWND, 900, NULL);
			break;
		}
		else RedrawWindow(m_tab.GetHwnd(), NULL, NULL, RDW_INVALIDATE);
		break;

	case GC_TABCHANGE:
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		PostMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);
		break;

	case GC_DROPPEDTAB:
		{
			int begin = (int)lParam;
			int end = (int)wParam;
			if (begin == end)
				break;

			TCITEM tci;
			tci.mask = TCIF_PARAM;
			TabCtrl_GetItem(m_tab.GetHwnd(), begin, &tci);
			SESSION_INFO *s = (SESSION_INFO*)tci.lParam;
			if (s) {
				m_tab.RemovePage(begin);
				AddPage(s);

				// fix the "fixed" positions
				int tabCount = m_tab.GetCount();
				for (int i = 0; i < tabCount; i++) {
					TabCtrl_GetItem(m_tab.GetHwnd(), i, &tci);
					s = (SESSION_INFO*)tci.lParam;
					if (s && s->hContact && db_get_w(s->hContact, s->pszModule, "TabPosition", 0) != 0)
						db_set_w(s->hContact, s->pszModule, "TabPosition", (WORD)(i + 1));
				}
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

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_TAB && ((LPNMHDR)lParam)->code == NM_RCLICK) {
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
			HMENU hSubMenu = GetSubMenu(g_hMenu, 3);
			TranslateMenu(hSubMenu);

			if (si) {
				WORD w = db_get_w(si->hContact, si->pszModule, "TabPosition", 0);
				if (w == 0)
					CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND | MF_UNCHECKED);
				else
					CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND | MF_CHECKED);
			}
			else CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND | MF_UNCHECKED);

			switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, tci.pt.x, tci.pt.y, 0, m_hwnd, NULL)) {
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
		break;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

static CTabbedWindow *pDialog = NULL;

void CTabbedWindow::OnDestroy()
{
	if (db_get_b(NULL, CHAT_MODULE, "SavePosition", 0)) {
		RECT rc;
		GetWindowRect(m_hwnd, &rc);
		db_set_dw(NULL, CHAT_MODULE, "roomx", rc.left);
		db_set_dw(NULL, CHAT_MODULE, "roomy", rc.top);
		db_set_dw(NULL, CHAT_MODULE, "roomwidth", rc.right - rc.left);
		db_set_dw(NULL, CHAT_MODULE, "roomheight", rc.bottom - rc.top);
	}

	pDialog = NULL;
}

void InitTabs()
{
	if (g_Settings.bTabsEnable && pDialog == NULL) {
		pDialog = new CTabbedWindow();
		pDialog->Create();
	}
	else if (g_Settings.bTabsEnable) 
		UninitTabs();
}

void UninitTabs()
{
	if (pDialog != NULL) {
		pDialog->Close();
		pDialog = NULL;
	}

	arSavedTabs.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

void ShowRoom(SESSION_INFO *si)
{
	if (!si)
		return;

	// Do we need to create a window?
	if (si->hWnd == NULL) {
		if (g_Settings.bTabsEnable) {
			if (pDialog == NULL) {
				pDialog = new CTabbedWindow();
				pDialog->Show();
			}
			pDialog->AddPage(si);
			PostMessage(pDialog->GetHwnd(), WM_SIZE, 0, 0);
		}
		else {
			CChatRoomDlg *pRoom = new CChatRoomDlg(si);
			pRoom->Show();
		}

		PostMessage(si->hWnd, WM_SIZE, 0, 0);
		if (si->iType != GCW_SERVER)
			SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
		else
			SendMessage(si->hWnd, GC_UPDATETITLE, 0, 0);
		SendMessage(si->hWnd, GC_REDRAWLOG, 0, 0);
		SendMessage(si->hWnd, GC_UPDATESTATUSBAR, 0, 0);
	}

	SetWindowLongPtr(si->hWnd, GWL_EXSTYLE, GetWindowLongPtr(si->hWnd, GWL_EXSTYLE) | WS_EX_APPWINDOW);

	if (IsIconic(si->hWnd))
		ShowWindow(si->hWnd, SW_NORMAL);
	ShowWindow(si->hWnd, SW_SHOW);
	SetForegroundWindow(si->hWnd);

	SendMessage(si->hWnd, WM_MOUSEACTIVATE, 0, 0);
	SetFocus(GetDlgItem(si->hWnd, IDC_MESSAGE));
}
