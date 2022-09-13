/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
Copyright (c) 2007 Artem Shpynov
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
#include "filter.h"

#define OPTSTATE_PREFIX "s_"

#define NEW_PAGE_TIMER       10011
#define FILTER_TIMEOUT_TIMER 10012

#define ALL_MODULES_FILTER LPGENW("<all modules>")
#define CORE_MODULES_FILTER LPGENW("<core modules>")

int LangpackOptionsInit(WPARAM, LPARAM);

HANDLE hOptionsInitEvent;
static class COptionsDlg *pOptionsDlg = nullptr;

// Thread for search keywords in dialogs
static uint8_t bSearchState = 0; // 0 - not executed; 1 - in progress; 2 - completed;
static bool bLoadingPrivateOptions = false;
static int FilterLoadProgress = 100;

struct OptionsPage : public OPTIONSDIALOGPAGE
{
	~OptionsPage()
	{
		mir_free(szTitle.a);
		mir_free(szGroup.a);
		mir_free(szTab.a);
		if ((DWORD_PTR)pszTemplate & 0xFFFF0000)
			mir_free((char*)pszTemplate);
	}
};

typedef OBJLIST<OptionsPage> OptionsPageList;

static BOOL CALLBACK BoldGroupTitlesEnumChildren(HWND hwnd, LPARAM lParam)
{
	wchar_t szClass[64];
	GetClassName(hwnd, szClass, _countof(szClass));

	if (!mir_wstrcmp(szClass, L"Button") && (GetWindowLongPtr(hwnd, GWL_STYLE) & 0x0F) == BS_GROUPBOX)
		SendMessage(hwnd, WM_SETFONT, lParam, 0);
	return TRUE;
}

static void ThemeDialogBackground(HWND hwnd, BOOL tabbed)
{
	EnableThemeDialogTexture(hwnd, (tabbed ? ETDT_ENABLE : ETDT_DISABLE) | ETDT_USETABTEXTURE);
}

static wchar_t* GetPluginName(HINSTANCE hInstance, wchar_t *buffer, int size)
{
	wchar_t tszModuleName[MAX_PATH];
	GetModuleFileName(hInstance, tszModuleName, _countof(tszModuleName));
	wchar_t *dllName = wcsrchr(tszModuleName, '\\');
	if (!dllName)
		dllName = tszModuleName;
	else
		dllName++;

	wcsncpy_s(buffer, size, dllName, _TRUNCATE);
	return buffer;
}

static LRESULT CALLBACK AeroPaintSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void AeroPaintControl(HWND hwnd, HDC hdc, UINT msg, LPARAM lpFlags)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	HDC tempDC = CreateCompatibleDC(hdc);

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = rc.right;
	bmi.bmiHeader.biHeight = -rc.bottom;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	uint8_t *pBits;
	HBITMAP hBmp = CreateDIBSection(tempDC, &bmi, DIB_RGB_COLORS, (void **)&pBits, nullptr, 0);
	if (hBmp && pBits) {
		HBITMAP hOldBmp = (HBITMAP)SelectObject(tempDC, hBmp);

		// paint
		SetPropA(hwnd, "Miranda.AeroRender.Active", (HANDLE)TRUE);
		mir_callNextSubclass(hwnd, AeroPaintSubclassProc, msg, (WPARAM)tempDC, lpFlags);
		SetPropA(hwnd, "Miranda.AeroRender.Active", (HANDLE)FALSE);

		// Fix alpha channel
		GdiFlush();
		for (int i = 0; i < rc.right*rc.bottom; i++, pBits += 4)
			if (!pBits[3])
				pBits[3] = 255;

		// Copy to output
		BitBlt(hdc, 0, 0, rc.right, rc.bottom, tempDC, 0, 0, SRCCOPY);
		SelectObject(tempDC, hOldBmp);
		DeleteObject(hBmp);
	}
	DeleteDC(tempDC);
}

static LRESULT CALLBACK AeroPaintSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CTLCOLOREDIT:
		if (!GetPropA((HWND)lParam, "Miranda.AeroRender.Active"))
			RedrawWindow((HWND)lParam, nullptr, nullptr, RDW_INVALIDATE);
		break;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_PRINT:
	case WM_PRINTCLIENT:
		AeroPaintControl(hwnd, (HDC)wParam, msg, lParam);
		return TRUE;

	case WM_DESTROY:
		RemovePropA(hwnd, "Miranda.AeroRender.Active");
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		AeroPaintControl(hwnd, hdc, WM_PRINT, PRF_CLIENT | PRF_NONCLIENT);
		EndPaint(hwnd, &ps);
		return TRUE;
	}
	return mir_callNextSubclass(hwnd, AeroPaintSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Stub for the old options (hinst + resource + window procedure + lParam)

class COptionPageDialog : public CDlgBase
{
	DLGPROC m_wndProc;
	LPARAM  m_lParam;

public:
	COptionPageDialog(CMPluginBase &pPlug, int idDialog, DLGPROC pProc, LPARAM lParam) :
		CDlgBase(pPlug, idDialog),
		m_wndProc(pProc),
		m_lParam(lParam)
	{
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_INITDIALOG)
			lParam = m_lParam;

		LRESULT res = m_wndProc(m_hwnd, msg, wParam, lParam);
		if (!res)
			res = CDlgBase::DlgProc(msg, wParam, lParam);

		return res;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// One option page

struct OptionsPageData : public MZeroedObject
{
	OptionsPageData(const OPTIONSDIALOGPAGE &src)
	{
		if (src.pszTemplate != nullptr) {
			CMPluginBase *p = (CMPluginBase*)src.pPlugin;
			pDialog = new COptionPageDialog(*p, (INT_PTR)src.pszTemplate, src.pfnDlgProc, src.dwInitParam);
		}
		else pDialog = src.pDialog;

		assert(pDialog != nullptr);

		flags = src.flags;
		pPlugin = src.pPlugin;

		if (src.flags & ODPF_UNICODE)
			ptszTitle = mir_wstrdup(src.szTitle.w);
		else
			ptszTitle = mir_a2u(src.szTitle.a);

		if (src.flags & ODPF_UNICODE)
			ptszGroup = mir_wstrdup(src.szGroup.w);
		else
			ptszGroup = mir_a2u(src.szGroup.a);

		if (src.flags & ODPF_UNICODE)
			ptszTab = mir_wstrdup(src.szTab.w);
		else
			ptszTab = mir_a2u(src.szTab.a);
  	}
	
	~OptionsPageData()
	{
		if (pDialog != nullptr)
			pDialog->Close();
	}

	CDlgBase *pDialog;
	HPLUGIN pPlugin;
	ptrW ptszTitle, ptszGroup, ptszTab;
	HTREEITEM hTreeItem;
	bool bChanged, bInsideTab;
	int height;
	int width;
	uint32_t flags;

	__forceinline HWND getHwnd() const { return (pDialog == nullptr) ? nullptr : pDialog->GetHwnd(); }
	__forceinline HINSTANCE getInst() const { return pDialog->GetInst(); }

	__forceinline wchar_t* getString(wchar_t *ptszStr)
	{
		if (flags & ODPF_DONTTRANSLATE)
			return ptszStr;
		return TranslateW_LP(ptszStr, pPlugin);
	}

	HWND CreateOptionWindow(HWND hWndParent) const
	{
		pDialog->SetParent(hWndParent);
		pDialog->Create();
		return pDialog->GetHwnd();
	}

	PageHash GetPluginPageHash() const
	{
		return mir_hashstrW(ptszGroup) + mir_hashstrW(ptszTitle) + mir_hashstrW(ptszTab);
	}

	void FindFilterStrings(int enableKeywordFiltering, int current, HWND hWndParent)
	{
		HWND hWnd = nullptr;
		if (enableKeywordFiltering) {
			if (current)
				hWnd = getHwnd();
			else {
				hWnd = CreateOptionWindow(hWndParent);
				ShowWindow(hWnd, SW_HIDE); // make sure it's hidden
			}
		}

		uint32_t key = GetPluginPageHash(); // get the plugin page hash

		wchar_t pluginName[MAX_PATH];
		GetPluginName(getInst(), pluginName, _countof(pluginName));
		auto *temp = GetPluginNameByInstance(getInst());
		GetDialogStrings(enableKeywordFiltering, key, pluginName, hWnd, getString(ptszGroup), getString(ptszTitle), getString(ptszTab), _A2T(temp));

		if (enableKeywordFiltering && !current)
			DestroyWindow(hWnd); // destroy the page, we're done with it
	}

	int MatchesFilter(wchar_t *szFilterString)
	{
		return ContainsFilterString(GetPluginPageHash(), szFilterString);
	}
};

static int CompareOPD(const OptionsPageData *p1, const OptionsPageData *p2)
{
	int res = mir_wstrcmp(p1->ptszGroup, p2->ptszGroup);
	if (!res)
		res = mir_wstrcmp(p1->ptszTitle, p2->ptszTitle);
	if (!res)
		res = mir_wstrcmp(p1->ptszTab, p2->ptszTab);
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options dialog

static LRESULT CALLBACK OptionsFilterSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message != WM_PAINT && message != WM_PRINT)
		return mir_callNextSubclass(hWnd, OptionsFilterSubclassProc, message, wParam, lParam);

	if (GetFocus() == hWnd || GetWindowTextLength(hWnd))
		return mir_callNextSubclass(hWnd, OptionsFilterSubclassProc, message, wParam, lParam);

	RECT rc;
	GetClientRect(hWnd, &rc);

	PAINTSTRUCT paint;
	HDC hdc = (message == WM_PAINT) ? BeginPaint(hWnd, &paint) : (HDC)wParam;

	wchar_t buf[255];
	if (bSearchState == 1 && FilterLoadProgress < 100 && FilterLoadProgress > 0)
		mir_snwprintf(buf, TranslateT("Loading... %d%%"), FilterLoadProgress);
	else
		mir_snwprintf(buf, TranslateT("Search"));

	bool bDrawnByTheme = false;

	int oldMode = SetBkMode(hdc, TRANSPARENT);

	HTHEME hTheme = OpenThemeData(hWnd, L"EDIT");
	if (hTheme) {
		if (IsThemeBackgroundPartiallyTransparent(hTheme, EP_EDITTEXT, ETS_NORMAL))
			DrawThemeParentBackground(hWnd, hdc, &rc);

		RECT rc2;
		GetThemeBackgroundContentRect(hTheme, hdc, EP_EDITTEXT, ETS_NORMAL, &rc, &rc2);
		rc2.top = 2 * rc.top - rc2.top;
		rc2.left = 2 * rc.left - rc2.left;
		rc2.bottom = 2 * rc.bottom - rc2.bottom;
		rc2.right = 2 * rc.right - rc2.right;
		DrawThemeBackground(hTheme, hdc, EP_EDITTEXT, ETS_NORMAL, &rc2, &rc);

		HFONT hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
		HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
		DrawThemeText(hTheme, hdc, EP_EDITTEXT, ETS_DISABLED, buf, -1, 0, 0, &rc);

		SelectObject(hdc, oldFont);
		CloseThemeData(hTheme);
		bDrawnByTheme = true;
	}

	SetBkMode(hdc, oldMode);

	if (!bDrawnByTheme) {
		HFONT hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
		HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
		SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
		FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
		oldMode = SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, buf, -1, &rc, 0);
		SetBkMode(hdc, oldMode);
		SelectObject(hdc, oldFont);
	}

	if (message == WM_PAINT)
		EndPaint(hWnd, &paint);

	return 0;
}

class COptionsDlg : public CDlgBase
{
	int m_currentPage;
	HTREEITEM m_hCurrentPage;
	LIST<OptionsPageData> m_arOpd, m_arDeleted, m_arInserted;
	RECT m_rcDisplay;
	RECT m_rcTab;
	HFONT m_hBoldFont;
	bool m_bInsideApply = false;
	wchar_t m_szFilterString[1024];

	const wchar_t *m_szCaption, *m_szGroup, *m_szPage, *m_szTab;
	const OptionsPageList &m_pages;

	CTimer m_timerRebuild, m_timerFilter;
	CCtrlTreeView m_pageTree;
	CCtrlCombo m_keywordFilter;
	CCtrlButton m_btnApply, m_btnCancel;

	COptionsDlg& operator=(const COptionsDlg&) = delete;

	HTREEITEM FindNamedTreeItem(HTREEITEM hParent, const wchar_t *name)
	{
		wchar_t str[128];
		TVITEMEX tvi;
		tvi.mask = TVIF_TEXT;
		tvi.pszText = str;
		tvi.cchTextMax = _countof(str);
		tvi.hItem = (hParent == nullptr) ? m_pageTree.GetRoot() : m_pageTree.GetChild(hParent);
		while (tvi.hItem != nullptr) {
			m_pageTree.GetItem(&tvi);
			if (!mir_wstrcmpi(str, name))
				return tvi.hItem;

			tvi.hItem = m_pageTree.GetNextSibling(tvi.hItem);
		}
		return nullptr;
	}

	void GetTreeSettingName(int idx, char *buf, size_t bufSize)
	{
		mir_snprintf(buf, bufSize, "%s%S", OPTSTATE_PREFIX, (idx < 0) ? m_arOpd[-idx]->ptszGroup.get() : m_arOpd[idx]->ptszTitle.get());
	}

	void SaveOptionsTreeState()
	{
		TVITEMEX tvi;
		tvi.mask = TVIF_STATE | TVIF_PARAM;
		for (tvi.hItem = m_pageTree.GetRoot(); tvi.hItem != nullptr; tvi.hItem = m_pageTree.GetNextSibling(tvi.hItem)) {
			if (m_pageTree.GetChild(tvi.hItem) == nullptr) continue;
			if (!m_pageTree.GetItem(&tvi)) continue;

			char buf[130];
			GetTreeSettingName(tvi.lParam, buf, _countof(buf));
			db_set_b(0, "Options", buf, (uint8_t)((tvi.state & TVIS_EXPANDED) ? 1 : 0));
		}
	}

	bool CheckPageShow(int i)
	{
		OptionsPageData *opd = m_arOpd[i];
		if (m_szFilterString[0] && !opd->MatchesFilter(m_szFilterString))
			return false;
		return true;
	}

	void FillFilterCombo()
	{
		HINSTANCE *KnownInstances = (HINSTANCE*)alloca(sizeof(HINSTANCE)*m_arOpd.getCount());
		int countKnownInst = 0;
		m_keywordFilter.ResetContent();
		m_keywordFilter.AddString(ALL_MODULES_FILTER, 0);
		m_keywordFilter.AddString(CORE_MODULES_FILTER, (LPARAM)g_plugin.getInst());

		for (auto &opd : m_arOpd) {
			opd->FindFilterStrings(false, 0, m_hwnd); // only modules name (fast enougth)

			HINSTANCE inst = opd->getInst();
			if (inst == g_plugin.getInst())
				continue;

			int j;
			for (j = 0; j < countKnownInst; j++)
				if (KnownInstances[j] == inst)
					break;
			if (j != countKnownInst)
				continue;

			KnownInstances[countKnownInst] = inst;
			countKnownInst++;

			wchar_t tszModuleName[MAX_PATH];
			GetModuleFileName(inst, tszModuleName, _countof(tszModuleName));

			wchar_t *dllName = mir_a2u(GetPluginNameByInstance(inst));
			if (!dllName) dllName = mir_wstrdup(wcsrchr(tszModuleName, '\\'));
			if (!dllName) dllName = mir_wstrdup(tszModuleName);
			if (dllName) {
				m_keywordFilter.AddString(dllName, (LPARAM)inst);
				mir_free(dllName);
			}
		}

		FilterLoadProgress = 100;
	}

	void CreateOptionWindowEx(OptionsPageData *opd)
	{
		opd->CreateOptionWindow(m_hwnd);
		if (opd->flags & ODPF_BOLDGROUPS)
			EnumChildWindows(opd->getHwnd(), BoldGroupTitlesEnumChildren, (LPARAM)m_hBoldFont);

		RECT rcPage;
		GetClientRect(opd->getHwnd(), &rcPage);
		int w = opd->width = rcPage.right;
		int h = opd->height = rcPage.bottom;

		RECT rc;
		GetWindowRect(opd->getHwnd(), &rc);

		opd->bInsideTab = IsInsideTab(m_currentPage);
		if (opd->bInsideTab) {
			SetWindowPos(opd->getHwnd(), HWND_TOP, (m_rcTab.left + m_rcTab.right - w) >> 1, m_rcTab.top, w, h, 0);
			::ThemeDialogBackground(opd->getHwnd(), TRUE);
		}
		else {
			SetWindowPos(opd->getHwnd(), HWND_TOP, (m_rcDisplay.left + m_rcDisplay.right - w) >> 1, (m_rcDisplay.top + m_rcDisplay.bottom - h) >> 1, w, h, 0);
			::ThemeDialogBackground(opd->getHwnd(), FALSE);
		}
	}

	void RebuildPageTree()
	{
		LPARAM oldSel = m_keywordFilter.SendMsg(CB_GETEDITSEL, 0, 0);
		m_keywordFilter.GetText(m_szFilterString, _countof(m_szFilterString));

		// if filter string is set to all modules then make the filter string empty (this will return all modules)
		BOOL bRemoveFocusFromFilter = FALSE;
		if (mir_wstrcmp(m_szFilterString, ALL_MODULES_FILTER) == 0) {
			m_szFilterString[0] = 0;
			bRemoveFocusFromFilter = TRUE;
		}
		// if filter string is set to core modules replace it with the name of the executable (this will return all core modules)
		else if (mir_wstrcmp(m_szFilterString, CORE_MODULES_FILTER) == 0) {
			// replace string with process name - that will show core settings
			wchar_t szFileName[300];
			GetModuleFileName(g_plugin.getInst(), szFileName, _countof(szFileName));
			wchar_t *pos = wcsrchr(szFileName, '\\');
			if (pos)
				pos++;
			else
				pos = szFileName;

			wcsncpy_s(m_szFilterString, pos, _TRUNCATE);
		}
		else {
			HINSTANCE hinst = (HINSTANCE)m_keywordFilter.GetCurData();
			if (hinst != INVALID_HANDLE_VALUE) {
				wchar_t szFileName[300];
				GetModuleFileName(hinst, szFileName, _countof(szFileName));
				wchar_t *pos = wcsrchr(szFileName, '\\');
				if (pos) pos++;
				else pos = szFileName;
				wcsncpy_s(m_szFilterString, pos, _TRUNCATE);
			}
		}

		_tcslwr_locale(m_szFilterString); //all strings are stored as lowercase ... make sure filter string is lowercase too

		m_pageTree.SetDraw(false);

		HWND oldWnd = nullptr;
		HWND oldTab = nullptr;
		CMStringW fullTitle;
		TVITEMEX tvi;

		OptionsPageData *opd = getCurrent();
		if (opd != nullptr) {
			oldWnd = opd->getHwnd();
			if (opd->bInsideTab)
				oldTab = GetDlgItem(m_hwnd, IDC_TAB);
		}

		m_hCurrentPage = nullptr;

		m_pageTree.SelectItem(nullptr);
		m_pageTree.DeleteAllItems();

		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = TVI_SORT;
		tvis.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
		tvis.item.state = tvis.item.stateMask = TVIS_EXPANDED;
		for (int i = 0; i < m_arOpd.getCount(); i++) {
			if (!CheckPageShow(i))
				continue;

			opd = m_arOpd[i];
			wchar_t *ptszGroup = TranslateW_LP(opd->ptszGroup, opd->pPlugin);
			wchar_t *ptszTab = TranslateW_LP(opd->ptszTab, opd->pPlugin);
			wchar_t *ptszTitle = opd->getString(opd->ptszTitle);

			tvis.hParent = nullptr;

			if (ptszGroup != nullptr) {
				tvis.hParent = FindNamedTreeItem(nullptr, ptszGroup);
				if (tvis.hParent == nullptr) {
					tvis.item.lParam = -i;
					tvis.item.pszText = ptszGroup;
					tvis.hParent = m_pageTree.InsertItem(&tvis);
				}
			}
			else {
				tvi.hItem = FindNamedTreeItem(nullptr, ptszTitle);
				if (tvi.hItem != nullptr) {
					if (i == m_currentPage)
						m_hCurrentPage = tvi.hItem;
					
					tvi.mask = TVIF_PARAM;
					m_pageTree.GetItem(&tvi);
					if (tvi.lParam < 0) {
						tvi.lParam = i;
						m_pageTree.SetItem(&tvi);
						continue;
					}
				}
			}

			if (ptszTab != nullptr) {
				HTREEITEM hItem;
				if (tvis.hParent == nullptr)
					hItem = FindNamedTreeItem(nullptr, ptszTitle);
				else
					hItem = FindNamedTreeItem(tvis.hParent, ptszTitle);
				if (hItem != nullptr) {
					if (i == m_currentPage)
						m_hCurrentPage = hItem;
					continue;
				}
			}

			tvis.item.pszText = ptszTitle;
			tvis.item.lParam = i;
			opd->hTreeItem = m_pageTree.InsertItem(&tvis);
			if (i == m_currentPage)
				m_hCurrentPage = opd->hTreeItem;
		}

		tvi.mask = TVIF_STATE | TVIF_PARAM;
		for (tvi.hItem = m_pageTree.GetRoot(); tvi.hItem != nullptr; tvi.hItem = m_pageTree.GetNextSibling(tvi.hItem)) {
			if (!m_pageTree.GetItem(&tvi)) continue;

			char buf[130];
			GetTreeSettingName(tvi.lParam, buf, _countof(buf));
			if (!db_get_b(0, "Options", buf, 1))
				m_pageTree.Expand(tvi.hItem, TVE_COLLAPSE);
		}

		if (m_hCurrentPage == nullptr) {
			m_hCurrentPage = m_pageTree.GetRoot();
			m_currentPage = -1;
		}
		m_pageTree.SelectItem(m_hCurrentPage);

		if (oldWnd) {
			opd = getCurrent();
			if (opd && oldWnd != opd->getHwnd()) {
				ShowWindow(oldWnd, SW_HIDE);
				if (oldTab && (opd == nullptr || !opd->bInsideTab))
					ShowWindow(oldTab, SW_HIDE);
			}
		}

		if (m_szFilterString[0] == 0) // Clear the keyword combo box
			m_keywordFilter.SetText(L"");
		if (!bRemoveFocusFromFilter)
			SetFocus(m_keywordFilter.GetHwnd()); //set the focus back to the combo box

		m_keywordFilter.SendMsg(CB_SETEDITSEL, 0, oldSel); //but don't select any of the text

		m_pageTree.SetDraw(true);
		m_pageTree.EnsureVisible(m_hCurrentPage);
	}

	bool IsInsideTab(int i)
	{
		OptionsPageData *opd = m_arOpd[i];
		int pages = 0;
		if (opd->ptszTab != nullptr) {
			// Count tabs to calc position
			for (int j = 0; j < m_arOpd.getCount() && pages < 2; j++) {
				OptionsPageData* opd2 = m_arOpd[j];
				if (!CheckPageShow(j)) continue;
				if (mir_wstrcmp(opd2->ptszTitle, opd->ptszTitle) || mir_wstrcmp(opd2->ptszGroup, opd->ptszGroup))
					continue;
				pages++;
			}
		}
		return (pages > 1);
	}

	OptionsPageData* getCurrent() const
	{	return (m_currentPage == -1) ? nullptr : m_arOpd[m_currentPage];
	}

public:
	COptionsDlg(const wchar_t *pszCaption, const wchar_t *pszGroup, const wchar_t *pszPage, const wchar_t *pszTab, bool bSinglePage, const OptionsPageList &arPages) :
		CDlgBase(g_plugin, bSinglePage ? IDD_OPTIONSPAGE : IDD_OPTIONS),
		m_btnApply(this, IDC_APPLY),
		m_btnCancel(this, IDCANCEL),
		m_pageTree(this, IDC_PAGETREE),
		m_keywordFilter(this, IDC_KEYWORD_FILTER),
		m_timerFilter(this, FILTER_TIMEOUT_TIMER),
		m_timerRebuild(this, NEW_PAGE_TIMER),
		m_arOpd(10),
		m_arDeleted(1),
		m_arInserted(1),
		m_szCaption(pszCaption),
		m_szGroup(pszGroup),
		m_szPage(pszPage),
		m_szTab(pszTab),
		m_pages(arPages)
	{
		m_keywordFilter.UseSystemColors();
		m_keywordFilter.OnChange = Callback(this, &COptionsDlg::OnFilterChanged);

		m_pageTree.OnSelChanging = Callback(this, &COptionsDlg::OnChanging);
		m_pageTree.OnSelChanged = Callback(this, &COptionsDlg::OnTreeChanged);

		m_btnCancel.OnClick = Callback(this, &COptionsDlg::OnCancel);
		m_btnApply.OnClick = Callback(this, &COptionsDlg::btnApply_Click);

		m_timerFilter.OnEvent = Callback(this, &COptionsDlg::onFilterTimer);
		m_timerRebuild.OnEvent = Callback(this, &COptionsDlg::onNewPageTimer);
	}

	bool OnInitDialog() override
	{
		Utils_RestoreWindowPositionNoSize(m_hwnd, 0, "Options", "");
		Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_OTHER_OPTIONS);
		m_btnApply.Disable();

		COMBOBOXINFO cbi;
		cbi.cbSize = sizeof(COMBOBOXINFO);
		GetComboBoxInfo(GetDlgItem(m_hwnd, IDC_KEYWORD_FILTER), &cbi);
		mir_subclassWindow(cbi.hwndItem, OptionsFilterSubclassProc);

		if (IsAeroMode()) {
			mir_subclassWindow(cbi.hwndCombo, AeroPaintSubclassProc);
			mir_subclassWindow(cbi.hwndItem, AeroPaintSubclassProc);
		}

		SetCaption(m_szCaption);

		LOGFONT lf;
		m_hBoldFont = (HFONT)m_btnApply.SendMsg(WM_GETFONT, 0, 0);
		GetObject(m_hBoldFont, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		m_hBoldFont = CreateFontIndirect(&lf);

		m_currentPage = -1;

		ptrW lastPage, lastGroup, lastTab;
		if (m_szPage == nullptr) {
			lastPage = db_get_wsa(0, "Options", "LastPage");

			if (m_szGroup == nullptr)
				lastGroup = db_get_wsa(0, "Options", "LastGroup");
			else
				lastGroup = mir_wstrdup(m_szGroup);
		}
		else {
			lastPage = mir_wstrdup(m_szPage);
			lastGroup = mir_wstrdup(m_szGroup);
		}

		if (m_szTab == nullptr)
			lastTab = db_get_wsa(0, "Options", "LastTab");
		else
			lastTab = mir_wstrdup(m_szTab);

		for (auto &it : m_pages) {
			OptionsPageData *opd = new OptionsPageData(*it);
			if (opd->pDialog == nullptr) // smth went wrong
				delete opd;
			else
				m_arOpd.insert(opd);

			if (!mir_wstrcmp(lastPage, it->szTitle.w) && !mir_wstrcmp(lastGroup, it->szGroup.w))
				if ((m_szTab == nullptr && m_currentPage == -1) || !mir_wstrcmp(lastTab, it->szTab.w))
					m_currentPage = m_pages.indexOf(&it);
		}

		GetWindowRect(GetDlgItem(m_hwnd, IDC_STNOPAGE), &m_rcDisplay);
		MapWindowPoints(nullptr, m_hwnd, (LPPOINT)&m_rcDisplay, 2);

		// Add an item to count in height
		TCITEM tie;
		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = -1;
		tie.pszText = L"X";
		TabCtrl_InsertItem(GetDlgItem(m_hwnd, IDC_TAB), 0, &tie);

		GetWindowRect(GetDlgItem(m_hwnd, IDC_TAB), &m_rcTab);
		MapWindowPoints(nullptr, m_hwnd, (LPPOINT)&m_rcTab, 2);
		TabCtrl_AdjustRect(GetDlgItem(m_hwnd, IDC_TAB), FALSE, &m_rcTab);

		FillFilterCombo();
		RebuildPageTree();
		return true;
	}

	void OnDestroy() override
	{
		ClearFilterStrings();
		m_szFilterString[0] = 0;

		SaveOptionsTreeState();
		Window_FreeIcon_IcoLib(m_hwnd);

		OptionsPageData *opd = getCurrent();
		if (opd) {
			if (opd->ptszTab)
				db_set_ws(0, "Options", "LastTab", opd->ptszTab);
			else
				db_unset(0, "Options", "LastTab");
			if (opd->ptszGroup)
				db_set_ws(0, "Options", "LastGroup", opd->ptszGroup);
			else
				db_unset(0, "Options", "LastGroup");
			db_set_ws(0, "Options", "LastPage", opd->ptszTitle);
		}
		else {
			db_unset(0, "Options", "LastTab");
			db_unset(0, "Options", "LastGroup");
			db_unset(0, "Options", "LastPage");
		}

		Utils_SaveWindowPosition(m_hwnd, 0, "Options", "");

		for (auto &p : m_arOpd)
			delete p;

		DeleteObject(m_hBoldFont);
		pOptionsDlg = nullptr;
	}

	bool OnApply() override
	{
		m_btnApply.Disable();
		SetFocus(m_pageTree.GetHwnd());

		OptionsPageData *opd = getCurrent();
		if (opd != nullptr) {
			PSHNOTIFY pshn = {};
			pshn.lParam = IDC_APPLY;
			pshn.hdr.code = PSN_KILLACTIVE;
			pshn.hdr.hwndFrom = opd->getHwnd();
			if (SendMessage(opd->getHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn))
				return false;
		}

		LIST<OptionsPageData> arChanged(10, CompareOPD);
		m_bInsideApply = true;

		PSHNOTIFY pshn = {};
		pshn.hdr.code = PSN_APPLY;
		for (auto &p : m_arOpd) {
			if (p == nullptr)
				continue;

			if (p->getHwnd() == nullptr || !p->bChanged)
				continue;

			arChanged.insert(p);
			p->bChanged = false;
			pshn.hdr.hwndFrom = p->getHwnd();
			if (SendMessage(p->getHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn) == PSNRET_INVALID_NOCHANGEPAGE) {
				m_hCurrentPage = p->hTreeItem;
				m_pageTree.SelectItem(m_hCurrentPage);
				if (opd)
					opd->pDialog->Hide();
				m_currentPage = m_arOpd.indexOf(&p);
				if (opd)
					opd->pDialog->Show();
				m_bInsideApply = false;
				return false;
			}
		}
		m_bInsideApply = false;

		for (auto &it : m_arInserted)
			m_arOpd.insert(it);
		m_arInserted.destroy();

		// send PSN_WIZFINISH once to last changed tab that belongs to the same group
		pshn.hdr.code = PSN_WIZFINISH;
		for (int i = 0; i < arChanged.getCount(); i++) {
			OptionsPageData *p = arChanged[i];
			if (p->ptszTab == nullptr || p->pDialog == nullptr)
				continue;

			// calculate last changed tab
			OptionsPageData *p2 = arChanged[i+1];
			if (p2 != nullptr && !mir_wstrcmp(p->ptszTitle, p2->ptszTitle) && !mir_wstrcmp(p->ptszGroup, p2->ptszGroup))
				continue;

			SendMessage(p->pDialog->GetHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn);
		}
		return true;
	}

	void btnApply_Click(CCtrlButton*)
	{
		OnApply();
		m_btnApply.Disable();
	}

	void OnOk(void*)
	{
		if (GetParent(GetFocus()) == GetDlgItem(m_hwnd, IDC_KEYWORD_FILTER))
			return;

		OnApply();
		Close();
	}

	void OnCancel(CCtrlButton*)
	{
		PSHNOTIFY pshn;
		pshn.hdr.idFrom = 0;
		pshn.lParam = 0;
		pshn.hdr.code = PSN_RESET;
		for (auto &p : m_arOpd) {
			if (p->getHwnd() == nullptr || !p->bChanged)
				continue;
			pshn.hdr.hwndFrom = p->getHwnd();
			SendMessage(p->getHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn);
		}
	}

	void OnChanging(CCtrlTreeView::TEventInfo*)
	{
		OptionsPageData *opd = getCurrent();
		if (opd && opd->getHwnd() != nullptr) {
			PSHNOTIFY pshn;
			pshn.hdr.code = PSN_KILLACTIVE;
			pshn.hdr.hwndFrom = m_arOpd[m_currentPage]->getHwnd();
			pshn.hdr.idFrom = 0;
			pshn.lParam = 0;
			if (SendMessage(m_arOpd[m_currentPage]->getHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn)) {
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
				return;
			}
		}
	}

	void OnFilterChanged(void*)
	{
		if (m_bInitialized)
			m_timerFilter.Start(400);
	}

	void OnTreeChanged(CCtrlTreeView::TEventInfo *evt)
	{
		ShowWindow(GetDlgItem(m_hwnd, IDC_STNOPAGE), SW_HIDE);

		OptionsPageData *opd = getCurrent();
		if (opd && opd->getHwnd() != nullptr)
			ShowWindow(opd->getHwnd(), SW_HIDE);

		TVITEMEX tvi;
		tvi.hItem = m_hCurrentPage = m_pageTree.GetSelection();
		if (tvi.hItem == nullptr) {
			ShowWindow(GetDlgItem(m_hwnd, IDC_TAB), SW_HIDE);
			return;
		}

		ShowWindow(GetDlgItem(m_hwnd, IDC_TAB), SW_HIDE);

		// if called inside OnInitDialog, we do not update current page
		if (IsInitialized()) {
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			m_pageTree.GetItem(&tvi);
			m_currentPage = tvi.lParam;
		}

		opd = getCurrent();
		if (opd == nullptr) {
			ShowWindow(GetDlgItem(m_hwnd, IDC_STNOPAGE), SW_SHOW);
			return;
		}

		if (opd->getHwnd() == nullptr)
			CreateOptionWindowEx(opd);

		opd->bInsideTab = IsInsideTab(m_currentPage);
		if (opd->bInsideTab) {
			// Make tabbed pane
			int pages = 0, sel = 0;
			HWND hwndTab = GetDlgItem(m_hwnd, IDC_TAB);
			TabCtrl_DeleteAllItems(hwndTab);

			TCITEM tie;
			tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
			tie.iImage = -1;
			for (int i = 0; i < m_arOpd.getCount(); i++) {
				if (!CheckPageShow(i))
					continue;

				OptionsPageData *p = m_arOpd[i];
				if (mir_wstrcmp(opd->ptszTitle, p->ptszTitle) || mir_wstrcmp(opd->ptszGroup, p->ptszGroup))
					continue;

				tie.pszText = TranslateW_LP(p->ptszTab, p->pPlugin);
				tie.lParam = i;
				TabCtrl_InsertItem(hwndTab, pages, &tie);
				if (!mir_wstrcmp(opd->ptszTab, p->ptszTab))
					sel = pages;
				pages++;
			}
			TabCtrl_SetCurSel(hwndTab, sel);
			ShowWindow(hwndTab, opd->bInsideTab ? SW_SHOW : SW_HIDE);
		}

		::ThemeDialogBackground(opd->getHwnd(), opd->bInsideTab);

		ShowWindow(opd->getHwnd(), SW_SHOW);
		if (evt->nmtv->action == TVC_BYMOUSE) {
			if (m_currentPage != -1)
				SetFocus(m_arOpd[m_currentPage]->getHwnd());
		}
		else SetFocus(m_pageTree.GetHwnd());
	}

	void OnTabChanged()
	{
		ShowWindow(GetDlgItem(m_hwnd, IDC_STNOPAGE), SW_HIDE);

		OptionsPageData *opd = getCurrent();
		if (opd && opd->getHwnd() != nullptr)
			ShowWindow(opd->getHwnd(), SW_HIDE);

		TCITEM tie;
		tie.mask = TCIF_PARAM;
		TabCtrl_GetItem(GetDlgItem(m_hwnd, IDC_TAB), TabCtrl_GetCurSel(GetDlgItem(m_hwnd, IDC_TAB)), &tie);
		m_currentPage = tie.lParam;

		TVITEMEX tvi;
		tvi.hItem = m_hCurrentPage;
		tvi.mask = TVIF_PARAM;
		tvi.lParam = m_currentPage;
		m_pageTree.SetItem(&tvi);

		opd = getCurrent();
		if (opd == nullptr) {
			ShowWindow(GetDlgItem(m_hwnd, IDC_STNOPAGE), SW_SHOW);
			return;
		}

		if (opd->getHwnd() == nullptr)
			CreateOptionWindowEx(opd);

		ShowWindow(opd->getHwnd(), SW_SHOW);
		SetFocus(m_pageTree.GetHwnd());
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case PSM_CHANGED:
			m_btnApply.Enable();
			{
				OptionsPageData *opd = getCurrent();
				if (opd)
					opd->bChanged = true;
			}
			return TRUE;

		case PSM_GETBOLDFONT:
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, (LONG_PTR)m_hBoldFont);
			return TRUE;

		case WM_NOTIFY:
			switch (wParam) {
			case IDC_TAB:
			case IDC_PAGETREE:
				switch (((LPNMHDR)lParam)->code) {
				case TVN_ITEMEXPANDING:
					SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, FALSE);
					return TRUE;

				case TCN_SELCHANGING:
					OnChanging(nullptr);
					return TRUE;

				case TCN_SELCHANGE:
					OnTabChanged();
					return TRUE;
				}
			}
			break;
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onFilterTimer(CTimer *pTimer)
	{
		pTimer->Stop();
		SaveOptionsTreeState();
		RebuildPageTree();
	}

	void onNewPageTimer(CTimer *pTimer)
	{
		pTimer->Stop();

		for (auto &it : m_arDeleted) {
			int idx = m_arOpd.indexOf(it);
			if (idx == -1)
				continue;

			if (m_currentPage > idx)
				m_currentPage--;

			delete it;
			m_arOpd.remove(idx);
		}

		RebuildPageTree();
	}

	void Locate(const wchar_t *pszGroup, const wchar_t *pszPage, HPLUGIN pPlugin)
	{
		ShowWindow(GetHwnd(), SW_RESTORE);
		SetForegroundWindow(m_hwnd);
		if (pszPage != nullptr) {
			HTREEITEM hItem = nullptr;
			if (pszGroup != nullptr) {
				hItem = FindNamedTreeItem(nullptr, TranslateW_LP(pszGroup, pPlugin));
				if (hItem != nullptr)
					hItem = FindNamedTreeItem(hItem, TranslateW_LP(pszPage, pPlugin));
			}
			else hItem = FindNamedTreeItem(nullptr, TranslateW_LP(pszPage, pPlugin));

			if (hItem != nullptr)
				m_pageTree.SelectItem(hItem);
		}
	}

	void DynamicAddPage(OptionsPage *pPage)
	{
		OptionsPageData *opd = new OptionsPageData(*pPage);
		if (opd->pDialog == nullptr) // smth went wrong
			delete opd;
		else {
			if (m_bInsideApply)
				m_arInserted.insert(opd);
			else
				m_arOpd.insert(opd);
			m_timerRebuild.Start(50);
		}
	}

	void KillModule(HPLUGIN pPlugin)
	{
		for (auto &opd : m_arOpd) {
			if (opd->pPlugin != pPlugin)
				continue;

			if (opd->pDialog != nullptr) {
				opd->pDialog->Close();
				opd->pDialog = nullptr;
			}

			m_arDeleted.insert(opd);
		}

		if (m_arDeleted.getCount())
			m_timerRebuild.Start(50);
	}
};

void OpenAccountOptions(PROTOACCOUNT *pa)
{
	if (pa->ppro == nullptr)
		return;

	OptionsPageList arPages(1);
	bLoadingPrivateOptions = true;
	CallObjectEventHook(pa->ppro, hOptionsInitEvent, (WPARAM)&arPages, 0);
	bLoadingPrivateOptions = false;
	if (arPages.getCount() == 0)
		return;

	wchar_t tszTitle[100];
	mir_snwprintf(tszTitle, TranslateT("%s options"), pa->tszAccountName);
	pOptionsDlg = new COptionsDlg(tszTitle, LPGENW("Network"), pa->tszAccountName, nullptr, true, arPages);
	pOptionsDlg->Show();
}

static void OpenOptionsNow(HPLUGIN pPlugin, const wchar_t *pszGroup, const wchar_t *pszPage, const wchar_t *pszTab, bool bSinglePage)
{
	// Hidden setting
	if (!db_get_b(0, "Options", "Enable", true)) {
		MessageBoxW(0,
			TranslateT("Options dialog is disabled by the system administrator. Contact him if you need to edit options"),
			L"Miranda NG", MB_ICONEXCLAMATION);
		return;
	}

	if (pOptionsDlg == nullptr) {
		OptionsPageList arPages(1);
		NotifyEventHooks(hOptionsInitEvent, (WPARAM)&arPages, 0);
		if (arPages.getCount() == 0)
			return;

		pOptionsDlg = new COptionsDlg(TranslateT("Miranda NG options"), pszGroup, pszPage, pszTab, bSinglePage, arPages);
		pOptionsDlg->Show();
	}
	else pOptionsDlg->Locate(pszGroup, pszPage, pPlugin);
}

MIR_APP_DLL(void) Options_Open(const wchar_t *pszGroup, const wchar_t *pszPage, const wchar_t *pszTab, HPLUGIN pPlugin)
{
	OpenOptionsNow(pPlugin, pszGroup, pszPage, pszTab, false);
}

MIR_APP_DLL(void) Options_OpenPage(const wchar_t *pszGroup, const wchar_t *pszPage, const wchar_t *pszTab, HPLUGIN pPlugin)
{
	OpenOptionsNow(pPlugin, pszGroup, pszPage, pszTab, true);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Options_AddPage(WPARAM wParam, OPTIONSDIALOGPAGE *odp, HPLUGIN pPlugin)
{
	OptionsPageList *pList = (OptionsPageList*)wParam;
	if (odp == nullptr)
		return 1;

	OptionsPage *dst = new OptionsPage();
	memcpy(dst, odp, sizeof(OPTIONSDIALOGPAGE));
	dst->pPlugin = pPlugin;

	if (odp->szTitle.w != nullptr) {
		if (odp->flags & ODPF_UNICODE)
			dst->szTitle.w = mir_wstrdup(odp->szTitle.w);
		else {
			dst->szTitle.w = mir_a2u(odp->szTitle.a);
			dst->flags |= ODPF_UNICODE;
		}
	}

	if (odp->szGroup.w != nullptr) {
		if (odp->flags & ODPF_UNICODE)
			dst->szGroup.w = mir_wstrdup(odp->szGroup.w);
		else {
			dst->szGroup.w = mir_a2u(odp->szGroup.a);
			dst->flags |= ODPF_UNICODE;
		}
	}

	if (odp->szTab.w != nullptr) {
		if (odp->flags & ODPF_UNICODE)
			dst->szTab.w = mir_wstrdup(odp->szTab.w);
		else {
			dst->szTab.w = mir_a2u(odp->szTab.a);
			dst->flags |= ODPF_UNICODE;
		}
	}

	if ((DWORD_PTR)odp->pszTemplate & 0xFFFF0000)
		dst->pszTemplate = mir_strdup(odp->pszTemplate);

	if (pList != nullptr)
		pList->insert(dst);

	if (pOptionsDlg && !bLoadingPrivateOptions)
		pOptionsDlg->DynamicAddPage(dst);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) KillModuleOptions(HPLUGIN pPlugin)
{
	if (pOptionsDlg != nullptr)
		pOptionsDlg->KillModule(pPlugin);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR OpenOptionsDialog(WPARAM, LPARAM)
{
	OpenOptionsNow(0, nullptr, nullptr, nullptr, false);
	return 0;
}

static int OptDynamicLoadOptions(WPARAM, LPARAM hInstance)
{
	OptionsPageList arPages(1);
	CallPluginEventHook((HINSTANCE)hInstance, ME_OPT_INITIALISE, (WPARAM)&arPages, 0);
	return 0;
}

int ShutdownOptionsModule(WPARAM, LPARAM)
{
	delete pOptionsDlg; pOptionsDlg = nullptr;
	return 0;
}

int LoadOptionsModule(void)
{
	CreateServiceFunction(MS_OPTIONS_OPEN, OpenOptionsDialog);

	HookEvent(ME_OPT_INITIALISE, LangpackOptionsInit);
	HookEvent(ME_SYSTEM_MODULELOAD, OptDynamicLoadOptions);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownOptionsModule);
	return 0;
}
