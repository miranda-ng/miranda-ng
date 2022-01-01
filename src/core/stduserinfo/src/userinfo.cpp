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

#define UPDATEANIMFRAMES 20

int DetailsInit(WPARAM wParam, LPARAM lParam);

static MWindowList hWindowList = nullptr;
static HANDLE hDetailsInitEvent;

struct DetailsPageInit
{
	int pageCount;
	OPTIONSDIALOGPAGE *odp;
};

struct DetailsPageData : public MNonCopyable
{
	DLGTEMPLATE *pTemplate;
	HINSTANCE    hInst;
	DLGPROC      dlgProc;
	LPARAM       dlgParam;
	HWND         hwnd;
	HTREEITEM    hItem;
	HPLUGIN      pPlugin;
	int          changed;
	uint32_t        dwFlags;
	wchar_t     *pwszTitle, *pwszTab;

	~DetailsPageData()
	{
		if (hwnd != nullptr)
			DestroyWindow(hwnd);
		mir_free(pwszTitle);
		mir_free(pwszTab);
	}

	__forceinline wchar_t *getTitle() const {
		return (dwFlags & ODPF_DONTTRANSLATE) ? pwszTitle : TranslateW_LP(pwszTitle, pPlugin);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// User info dialog

static void ThemeDialogBackground(HWND hwnd)
{
	EnableThemeDialogTexture(hwnd, ETDT_ENABLETAB);
}

static int UserInfoContactDelete(WPARAM wParam, LPARAM)
{
	HWND hwnd = WindowList_Find(hWindowList, wParam);
	if (hwnd != nullptr)
		DestroyWindow(hwnd);
	return 0;
}

#define M_PROTOACK    (WM_USER+10)
#define M_CHECKONLINE (WM_USER+11)
#define M_DLLUNLOAD   (WM_USER+12)

class CUserInfoDlg : public CDlgBase
{
	MCONTACT  m_hContact;
	HINSTANCE m_hInstIcmp = 0;
	HFONT     m_hBoldFont = 0;
	int       m_currentPage = -1;
	RECT      m_rcDisplay, m_rcDisplayTab;
	int       m_updateAnimFrame = 0;
	wchar_t   m_szUpdating[64];
	int      *m_infosUpdated = 0;

	HANDLE    m_hProtoAckEvent = 0;
	HANDLE    m_hDllUnloadEvent = 0;

	HWND      m_tabs;
	OBJLIST<DetailsPageData> m_pages;

	void BuildTree()
	{
		ptrW ptszLastTab(g_plugin.getWStringA("LastTab"));
		m_currentPage = -1;

		for (int i = 0; i < m_pages.getCount(); i++) {
			auto &p = m_pages[i];
			if (i && p.pwszTab && !mir_wstrcmp(m_pages[i - 1].pwszTitle, p.pwszTitle)) {
				p.hItem = m_pages[i - 1].hItem;
				continue;
			}

			TVINSERTSTRUCT tvis;
			tvis.hParent = nullptr;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvis.item.lParam = (LPARAM)i;
			tvis.item.pszText = p.getTitle();
			if (ptszLastTab && !mir_wstrcmp(tvis.item.pszText, ptszLastTab))
				m_currentPage = i;
			p.hItem = m_tree.InsertItem(&tvis);
		}

		if (m_currentPage == -1)
			m_currentPage = 0;
	}

	void CreateDetailsTabs(DetailsPageData *ppg)
	{
		int sel = 0, pages = 0;

		TCITEM tie;
		tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
		tie.iImage = -1;
		TabCtrl_DeleteAllItems(m_tabs);

		for (auto &odp : m_pages) {
			if (!odp->pwszTab || mir_wstrcmp(odp->pwszTitle, ppg->pwszTitle))
				continue;

			tie.pszText = TranslateW_LP(odp->pwszTab, odp->pPlugin);
			tie.lParam = m_pages.indexOf(&odp);
			TabCtrl_InsertItem(m_tabs, pages, &tie);
			if (!mir_wstrcmp(odp->pwszTab, ppg->pwszTab))
				sel = pages;
			pages++;
		}
		TabCtrl_SetCurSel(m_tabs, sel);

		LONG style = GetWindowLongPtr(m_tabs, GWL_STYLE);
		SetWindowLongPtr(m_tabs, GWL_STYLE, pages > 1 ? style | WS_TABSTOP : style & ~WS_TABSTOP);
	}

	void CreateDetailsPageWindow(DetailsPageData *ppg)
	{
		RECT *rc = ppg->pwszTab ? &m_rcDisplayTab : &m_rcDisplay;
		ppg->hwnd = CreateDialogIndirectParam(ppg->hInst, ppg->pTemplate, m_hwnd, ppg->dlgProc, (LPARAM)m_hContact);
		::ThemeDialogBackground(ppg->hwnd);
		SetWindowPos(ppg->hwnd, HWND_TOP, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, 0);
		SetWindowPos(ppg->hwnd, HWND_TOP, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, 0);

		PSHNOTIFY pshn;
		pshn.hdr.code = PSN_PARAMCHANGED;
		pshn.hdr.hwndFrom = ppg->hwnd;
		pshn.hdr.idFrom = 0;
		pshn.lParam = (LPARAM)ppg->dlgParam;
		SendMessage(ppg->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);

		pshn.hdr.code = PSN_INFOCHANGED;
		pshn.hdr.hwndFrom = ppg->hwnd;
		pshn.hdr.idFrom = 0;
		pshn.lParam = (LPARAM)m_hContact;
		SendMessage(ppg->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);
	}

	CCtrlBase m_white;
	CCtrlTreeView m_tree;
	CTimer updateTimer;

public:
	CUserInfoDlg(MCONTACT hContact, int count, OPTIONSDIALOGPAGE *odp) :
		CDlgBase(g_plugin, IDD_DETAILS),
		m_hContact(hContact),
		m_pages(1),
		m_tree(this, IDC_PAGETREE),
		m_white(this, IDC_WHITERECT),
		updateTimer(this, 1)
	{
		m_white.UseSystemColors();

		m_tree.OnSelChanged = Callback(this, &CUserInfoDlg::onSelChanged_Tree);
		m_tree.OnSelChanging = Callback(this, &CUserInfoDlg::onSelChanging);

		updateTimer.OnEvent = Callback(this, &CUserInfoDlg::onTimer);

		for (int i = 0; i < count; i++, odp++) {
			HINSTANCE hInst = odp->pPlugin->getInst();
			if (hInst != nullptr) {
				auto *p = new DetailsPageData();
				p->pTemplate = (LPDLGTEMPLATE)LockResource(LoadResource(hInst, FindResourceA(hInst, odp->pszTemplate, MAKEINTRESOURCEA(5))));
				p->dlgProc = odp->pfnDlgProc;
				p->dlgParam = odp->dwInitParam;
				p->hInst = hInst;
				p->dwFlags = odp->flags;

				p->pwszTitle = odp->szTitle.w;
				p->pwszTab = odp->szTab.w;
				p->pPlugin = odp->pPlugin;
				m_pages.insert(p);
			}

			if (odp->szGroup.a != nullptr)
				mir_free(odp->szGroup.a);
			if ((DWORD_PTR)odp->pszTemplate & 0xFFFF0000)
				mir_free((char *)odp->pszTemplate);
		}
	}

	bool OnInitDialog() override
	{
		Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_OTHER_USERDETAILS);

		m_hProtoAckEvent = HookEventMessage(ME_PROTO_ACK, m_hwnd, M_PROTOACK);
		m_hDllUnloadEvent = HookEventMessage(ME_SYSTEM_MODULEUNLOAD, m_hwnd, M_DLLUNLOAD);
		WindowList_Add(hWindowList, m_hwnd, m_hContact);

		//////////////////////////////////////////////////////////////////////
		wchar_t *name, oldTitle[256], newTitle[256];
		if (m_hContact == NULL)
			name = TranslateT("Owner");
		else
			name = Clist_GetContactDisplayName(m_hContact);

		GetWindowText(m_hwnd, oldTitle, _countof(oldTitle));
		mir_snwprintf(newTitle, oldTitle, name);
		SetWindowText(m_hwnd, newTitle);

		//////////////////////////////////////////////////////////////////////
		LOGFONT lf;
		HFONT hNormalFont = (HFONT)SendDlgItemMessage(m_hwnd, IDC_NAME, WM_GETFONT, 0, 0);
		GetObject(hNormalFont, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		m_hBoldFont = CreateFontIndirect(&lf);
		SendDlgItemMessage(m_hwnd, IDC_NAME, WM_SETFONT, (WPARAM)m_hBoldFont, 0);

		BuildTree();

		//////////////////////////////////////////////////////////////////////
		m_tabs = GetDlgItem(m_hwnd, IDC_TABS);

		TCITEM tci;
		tci.mask = TCIF_TEXT | TCIF_IMAGE;
		tci.iImage = -1;
		tci.pszText = L"X";
		TabCtrl_InsertItem(m_tabs, 0, &tci);

		GetWindowRect(m_tabs, &m_rcDisplayTab);
		TabCtrl_AdjustRect(m_tabs, FALSE, &m_rcDisplayTab);

		POINT pt = { 0, 0 };
		ClientToScreen(m_hwnd, &pt);
		OffsetRect(&m_rcDisplayTab, -pt.x, -pt.y);

		TabCtrl_DeleteAllItems(m_tabs);

		GetWindowRect(m_tabs, &m_rcDisplay);
		TabCtrl_AdjustRect(m_tabs, FALSE, &m_rcDisplay);

		memset(&pt, 0, sizeof(pt));
		ClientToScreen(m_hwnd, &pt);
		OffsetRect(&m_rcDisplay, -pt.x, -pt.y);

		m_tree.SelectItem(m_pages[m_currentPage].hItem);

		//////////////////////////////////////////////////////////////////////
		m_updateAnimFrame = 0;
		GetDlgItemText(m_hwnd, IDC_UPDATING, m_szUpdating, _countof(m_szUpdating));
		SendMessage(m_hwnd, M_CHECKONLINE, 0, 0);
		if (!ProtoChainSend(m_hContact, PSS_GETINFO, SGIF_ONOPEN, 0)) {
			EnableWindow(GetDlgItem(m_hwnd, IDC_UPDATE), FALSE);
			SetTimer(m_hwnd, 1, 100, nullptr);
		}
		else ShowWindow(GetDlgItem(m_hwnd, IDC_UPDATING), SW_HIDE);

		SetFocus(m_tree.GetHwnd());
		return true;
	}

	bool OnApply() override
	{
		PSHNOTIFY pshn;
		pshn.hdr.idFrom = 0;
		pshn.lParam = (LPARAM)m_hContact;
		if (m_currentPage != -1) {
			pshn.hdr.code = PSN_KILLACTIVE;
			pshn.hdr.hwndFrom = m_pages[m_currentPage].hwnd;
			if (SendMessage(m_pages[m_currentPage].hwnd, WM_NOTIFY, 0, (LPARAM)&pshn))
				return false;
		}

		pshn.hdr.code = PSN_APPLY;
		for (auto &odp : m_pages) {
			if (odp->hwnd == nullptr || !odp->changed)
				continue;
			pshn.hdr.hwndFrom = odp->hwnd;
			if (SendMessage(odp->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn) == PSNRET_INVALID_NOCHANGEPAGE) {
				m_tree.Select(odp->hItem, TVGN_CARET);
				if (m_currentPage != -1) ShowWindow(m_pages[m_currentPage].hwnd, SW_HIDE);
				m_currentPage = m_pages.indexOf(&odp);
				ShowWindow(m_pages[m_currentPage].hwnd, SW_SHOW);
				return false;
			}
		}
		return true;
	}

	void OnDestroy() override
	{
		wchar_t name[128];
		TVITEMEX tvi;
		tvi.mask = TVIF_TEXT;
		tvi.hItem = m_pages[m_currentPage].hItem;
		tvi.pszText = name;
		tvi.cchTextMax = _countof(name);
		m_tree.GetItem(&tvi);
		g_plugin.setWString("LastTab", name);

		Window_FreeIcon_IcoLib(m_hwnd);
		SendDlgItemMessage(m_hwnd, IDC_NAME, WM_SETFONT, SendDlgItemMessage(m_hwnd, IDC_WHITERECT, WM_GETFONT, 0, 0), 0);
		DeleteObject(m_hBoldFont);
		WindowList_Remove(hWindowList, m_hwnd);

		UnhookEvent(m_hProtoAckEvent);
		UnhookEvent(m_hDllUnloadEvent);

		m_pages.destroy();
		mir_free(m_infosUpdated);
	}

	INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		PSHNOTIFY pshn;

		switch (uMsg) {
		case PSM_CHANGED:
			m_pages[m_currentPage].changed = 1;
			return TRUE;

		case PSM_FORCECHANGED:
			pshn.hdr.code = PSN_INFOCHANGED;
			pshn.hdr.idFrom = 0;
			pshn.lParam = (LPARAM)m_hContact;
			for (auto &odp : m_pages) {
				pshn.hdr.hwndFrom = odp->hwnd;
				if (odp->hwnd != nullptr)
					SendMessage(odp->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);
			}
			break;

		case M_CHECKONLINE:
			if (m_hContact != NULL) {
				char *szProto = Proto_GetBaseAccountName(m_hContact);
				if (szProto == nullptr)
					EnableWindow(GetDlgItem(m_hwnd, IDC_UPDATE), FALSE);
				else {
					if (Proto_GetStatus(szProto) < ID_STATUS_ONLINE)
						EnableWindow(GetDlgItem(m_hwnd, IDC_UPDATE), FALSE);
					else
						EnableWindow(GetDlgItem(m_hwnd, IDC_UPDATE), !IsWindowVisible(GetDlgItem(m_hwnd, IDC_UPDATING)));
				}
			}
			break;

		case M_DLLUNLOAD:
			{
				bool bRemoved = false;
				HINSTANCE hInst = (HINSTANCE)lParam;
				for (auto &odp : m_pages.rev_iter()) {
					if (odp->hInst == hInst) {
						if (!bRemoved) {
							m_tree.DeleteAllItems();
							bRemoved = true;
						}
						m_pages.removeItem(&odp);
					}
				}

				if (bRemoved)
					BuildTree();
			}

		case M_PROTOACK:
			{
				ACKDATA *ack = (ACKDATA *)lParam;
				if (ack->hContact == NULL && ack->type == ACKTYPE_STATUS) {
					SendMessage(m_hwnd, M_CHECKONLINE, 0, 0);
					break;
				}
				if (ack->hContact != m_hContact || ack->type != ACKTYPE_GETINFO)
					break;

				SendMessage(m_hwnd, PSM_FORCECHANGED, 0, 0);
				/* if they're not gonna send any more ACK's don't let that mean we should crash */
				if (!ack->hProcess && !ack->lParam) {
					ShowWindow(GetDlgItem(m_hwnd, IDC_UPDATING), SW_HIDE);
					updateTimer.Stop();
					SendMessage(m_hwnd, M_CHECKONLINE, 0, 0);
					break;
				} //if
				if (m_infosUpdated == nullptr)
					m_infosUpdated = (int *)mir_calloc(sizeof(int) * (INT_PTR)ack->hProcess);
				if (ack->result == ACKRESULT_SUCCESS || ack->result == ACKRESULT_FAILED)
					m_infosUpdated[ack->lParam] = 1;

				int i;
				for (i = 0; i < (INT_PTR)ack->hProcess; i++)
					if (m_infosUpdated[i] == 0)
						break;

				if (i == (INT_PTR)ack->hProcess) {
					ShowWindow(GetDlgItem(m_hwnd, IDC_UPDATING), SW_HIDE);
					updateTimer.Stop();
					SendMessage(m_hwnd, M_CHECKONLINE, 0, 0);
				}
			}
			break;
		
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code) {
			case TCN_SELCHANGING:
				onSelChanging(0);
				break;

			case TCN_SELCHANGE:
				if (m_currentPage != -1 && m_pages[m_currentPage].hwnd != NULL) {
					ShowWindow(m_pages[m_currentPage].hwnd, SW_HIDE);

					TCITEM tie;
					tie.mask = TCIF_PARAM;
					TabCtrl_GetItem(m_tabs, TabCtrl_GetCurSel(m_tabs), &tie);
					m_currentPage = tie.lParam;

					TVITEMEX tvi;
					tvi.hItem = m_tree.GetNextItem(NULL, TVGN_CARET);
					tvi.mask = TVIF_PARAM;
					tvi.lParam = m_currentPage;
					m_tree.SetItem(&tvi);

					if (m_currentPage != -1) {
						if (m_pages[m_currentPage].hwnd == NULL)
							CreateDetailsPageWindow(&m_pages[m_currentPage]);
						ShowWindow(m_pages[m_currentPage].hwnd, SW_SHOWNA);
					}
				}
				break;
			}
		}

		return CDlgBase::DlgProc(uMsg, wParam, lParam);
	}

	void onSelChanging(CCtrlTreeView *)
	{
		if (m_currentPage != -1 && m_pages[m_currentPage].hwnd != NULL) {
			PSHNOTIFY pshn;
			pshn.hdr.code = PSN_KILLACTIVE;
			pshn.hdr.hwndFrom = m_pages[m_currentPage].hwnd;
			pshn.hdr.idFrom = 0;
			pshn.lParam = (LPARAM)m_hContact;
			if (SendMessage(m_pages[m_currentPage].hwnd, WM_NOTIFY, 0, (LPARAM)&pshn))
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
		}
	}

	void onSelChanged_Tree(CCtrlTreeView::TEventInfo *ev)
	{
		if (m_currentPage != -1 && m_pages[m_currentPage].hwnd != NULL)
			ShowWindow(m_pages[m_currentPage].hwnd, SW_HIDE);

		LPNMTREEVIEW pnmtv = ev->nmtv;
		TVITEM tvi = pnmtv->itemNew;
		m_currentPage = tvi.lParam;

		if (m_currentPage != -1) {
			CreateDetailsTabs(&m_pages[m_currentPage]);
			if (m_pages[m_currentPage].hwnd == NULL)
				CreateDetailsPageWindow(&m_pages[m_currentPage]);
			ShowWindow(m_pages[m_currentPage].hwnd, SW_SHOWNA);
		}
	}

	void onClick_Cancel(CCtrlButton *)
	{
		PSHNOTIFY pshn;
		pshn.hdr.idFrom = 0;
		pshn.hdr.code = PSN_RESET;
		pshn.lParam = (LPARAM)m_hContact;

		for (auto &odp : m_pages) {
			if (odp->hwnd == nullptr || !odp->changed)
				continue;
			pshn.hdr.hwndFrom = odp->hwnd;
			SendMessage(odp->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);
		}
	}

	void onClick_Update(CCtrlButton *)
	{
		if (m_infosUpdated != NULL) {
			mir_free(m_infosUpdated);
			m_infosUpdated = NULL;
		}

		if (m_hContact != NULL) {
			if (!ProtoChainSend(m_hContact, PSS_GETINFO, 0, 0)) {
				EnableWindow(GetDlgItem(m_hwnd, IDC_UPDATE), FALSE);
				ShowWindow(GetDlgItem(m_hwnd, IDC_UPDATING), SW_SHOW);
				updateTimer.Start(100);
			}
		}
	}

	void onTimer(CTimer *)
	{
		wchar_t str[128];
		mir_snwprintf(str, L"%.*s%s%.*s", m_updateAnimFrame % 10, L".........", m_szUpdating, m_updateAnimFrame % 10, L".........");
		SetDlgItemText(m_hwnd, IDC_UPDATING, str);
		if (++m_updateAnimFrame == UPDATEANIMFRAMES)
			m_updateAnimFrame = 0;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Services

static INT_PTR AddDetailsPage(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE *odp = (OPTIONSDIALOGPAGE *)lParam;
	struct DetailsPageInit *opi = (struct DetailsPageInit *)wParam;

	if (odp == nullptr || opi == nullptr)
		return 1;

	opi->odp = (OPTIONSDIALOGPAGE *)mir_realloc(opi->odp, sizeof(OPTIONSDIALOGPAGE) * (opi->pageCount + 1));
	OPTIONSDIALOGPAGE *dst = opi->odp + opi->pageCount;
	memset(dst, 0, sizeof(OPTIONSDIALOGPAGE));
	dst->pPlugin = odp->pPlugin;
	dst->pfnDlgProc = odp->pfnDlgProc;
	dst->position = odp->position;
	dst->pszTemplate = ((DWORD_PTR)odp->pszTemplate & 0xFFFF0000) ? mir_strdup(odp->pszTemplate) : odp->pszTemplate;

	if (odp->flags & ODPF_UNICODE) {
		dst->szTitle.w = (odp->szTitle.w == nullptr) ? nullptr : mir_wstrdup(odp->szTitle.w);
		dst->szTab.w = (odp->flags & ODPF_USERINFOTAB) ? mir_wstrdup(odp->szTab.w) : nullptr;
	}
	else {
		dst->szTitle.w = mir_a2u(odp->szTitle.a);
		dst->szTab.w = (odp->flags & ODPF_USERINFOTAB) ? mir_a2u(odp->szTab.a) : nullptr;
	}

	dst->flags = odp->flags;
	dst->dwInitParam = odp->dwInitParam;
	opi->pageCount++;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t *getTab(OPTIONSDIALOGPAGE *p)
{
	return (p->flags & ODPF_DONTTRANSLATE) ? p->szTab.w : TranslateW_LP(p->szTab.w, p->pPlugin);
}

static wchar_t *getTitle(OPTIONSDIALOGPAGE *p)
{
	return (p->flags & ODPF_DONTTRANSLATE) ? p->szTitle.w : TranslateW_LP(p->szTitle.w, p->pPlugin);
}

static int PageSortProc(OPTIONSDIALOGPAGE *item1, OPTIONSDIALOGPAGE *item2)
{
	int res;
	wchar_t *s1 = getTitle(item1), *s2 = getTitle(item2);
	if (!mir_wstrcmp(s1, TranslateT("Summary"))) return -1;
	if (!mir_wstrcmp(s2, TranslateT("Summary"))) return 1;
	if (res = mir_wstrcmp(s1, s2)) return res;

	s1 = getTab(item1), s2 = getTab(item2);
	if (s1 && !s2) return -1;
	if (!s1 && s2) return 1;
	if (!s1 && !s2) return 0;

	if (s1 && !mir_wstrcmp(s1, TranslateT("General"))) return -1;
	if (s2 && !mir_wstrcmp(s2, TranslateT("General"))) return 1;
	return mir_wstrcmp(s1, s2);
}

static INT_PTR ShowDetailsDialogCommand(WPARAM hContact, LPARAM)
{
	if (HWND hwnd = WindowList_Find(hWindowList, hContact)) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
		return 0;
	}

	DetailsPageInit opi = {};
	NotifyEventHooks(hDetailsInitEvent, (WPARAM)&opi, hContact);
	if (opi.pageCount == 0)
		return 0;

	qsort(opi.odp, opi.pageCount, sizeof(OPTIONSDIALOGPAGE), (int(*)(const void *, const void *))PageSortProc);

	auto *pDlg = new CUserInfoDlg(hContact, opi.pageCount, opi.odp);
	pDlg->Show();

	mir_free(opi.odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module events

static int ShutdownUserInfo(WPARAM, LPARAM)
{
	WindowList_Broadcast(hWindowList, WM_DESTROY, 0, 0);
	WindowList_Destroy(hWindowList);
	return 0;
}

static int OnTopToolBarLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_USERINFO_SHOWDIALOG;
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS);
	ttb.name = ttb.pszTooltipUp = LPGEN("User &details");
	g_plugin.addTTB(&ttb);
	return 0;
}

int LoadUserInfoModule(void)
{
	CreateServiceFunction("UserInfo/AddPage", AddDetailsPage);
	CreateServiceFunction(MS_USERINFO_SHOWDIALOG, ShowDetailsDialogCommand);

	hDetailsInitEvent = CreateHookableEvent(ME_USERINFO_INITIALISE);

	HookEvent(ME_TTB_MODULELOADED, OnTopToolBarLoaded);
	HookEvent(ME_USERINFO_INITIALISE, DetailsInit);
	HookEvent(ME_DB_CONTACT_DELETED, UserInfoContactDelete);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownUserInfo);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xe8731d53, 0x95af, 0x42cf, 0xae, 0x27, 0xc7, 0xa7, 0xa, 0xbf, 0x14, 0x1c);
	mi.position = 1000050000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS);
	mi.name.a = LPGEN("User &details");
	mi.pszService = MS_USERINFO_SHOWDIALOG;
	Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x42852ca4, 0x4941, 0x4219, 0x8b, 0x98, 0x33, 0x46, 0x8c, 0x32, 0xd8, 0xb8);
	mi.position = 500050000;
	mi.name.a = LPGEN("View/change my &details...");
	Menu_AddMainMenuItem(&mi);

	hWindowList = WindowList_Create();
	return 0;
}
