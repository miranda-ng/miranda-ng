/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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

#define FILTER_TIMEOUT_TIMER 10012

#define HM_MODULELOAD (WM_USER+12)
#define HM_MODULEUNLOAD (WM_USER+13)

#define ALL_MODULES_FILTER LPGENW("<all modules>")
#define CORE_MODULES_FILTER LPGENW("<core modules>")

int LangpackOptionsInit(WPARAM, LPARAM);

static HANDLE hOptionsInitEvent;
static class COptionsDlg *pOptionsDlg = nullptr;

// Thread for search keywords in dialogs
static BYTE bSearchState = 0; // 0 - not executed; 1 - in progress; 2 - completed;
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

static BOOL IsAeroMode()
{
	BOOL result;
	return dwmIsCompositionEnabled && (dwmIsCompositionEnabled(&result) == S_OK) && result;
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

	BYTE *pBits;
	HBITMAP hBmp = CreateDIBSection(tempDC, &bmi, DIB_RGB_COLORS, (void **)&pBits, nullptr, 0);
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
	COptionPageDialog(HINSTANCE hInst, int idDialog, DLGPROC pProc, LPARAM lParam) :
		CDlgBase(hInst, idDialog),
		m_wndProc(pProc),
		m_lParam(lParam)
	{
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
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
		if (src.hInstance != nullptr && src.pszTemplate != nullptr)
			pDialog = new COptionPageDialog(src.hInstance, (INT_PTR)src.pszTemplate, src.pfnDlgProc, src.dwInitParam);
		else
			pDialog = src.pDialog;
		assert(pDialog != nullptr);

		flags = src.flags;
		hLangpack = src.hLangpack;

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
		delete pDialog;
	}

	CDlgBase *pDialog;
	int hLangpack;
	ptrW ptszTitle, ptszGroup, ptszTab;
	HTREEITEM hTreeItem;
	int changed;
	int height;
	int width;
	DWORD flags;
	BOOL insideTab;

	__forceinline HWND getHwnd() const { return pDialog->GetHwnd(); }
	__forceinline HINSTANCE getInst() const { return pDialog->GetInst(); }

	__forceinline wchar_t* getString(wchar_t *ptszStr)
	{
		if (flags & ODPF_DONTTRANSLATE)
			return ptszStr;
		return TranslateW_LP(ptszStr, hLangpack);
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
		HWND hWnd = 0;
		if (enableKeywordFiltering) {
			if (current)
				hWnd = getHwnd();
			else {
				hWnd = CreateOptionWindow(hWndParent);
				ShowWindow(hWnd, SW_HIDE); // make sure it's hidden
			}
		}

		DWORD key = GetPluginPageHash(); // get the plugin page hash

		wchar_t pluginName[MAX_PATH];
		char *temp = GetPluginNameByInstance(getInst());
		GetDialogStrings(enableKeywordFiltering, key, GetPluginName(getInst(), pluginName, _countof(pluginName)), hWnd, ptszGroup, ptszTitle, ptszTab, _A2T(temp));

		if (enableKeywordFiltering && !current)
			DestroyWindow(hWnd); // destroy the page, we're done with it
	}

	int MatchesFilter(wchar_t *szFilterString)
	{
		return ContainsFilterString(GetPluginPageHash(), szFilterString);
	}
};

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
	LIST<OptionsPageData> m_arOpd;
	RECT m_rcDisplay;
	RECT m_rcTab;
	HFONT m_hBoldFont;
	wchar_t m_szFilterString[1024];
	HANDLE m_hPluginLoad, m_hPluginUnload;

	const wchar_t *m_szCaption, *m_szGroup, *m_szPage, *m_szTab;
	const OptionsPageList &m_pages;

	CCtrlTreeView m_pageTree;
	CCtrlCombo m_keywordFilter;
	CCtrlButton m_btnApply, m_btnCancel;

	COptionsDlg& operator=(const COptionsDlg&);

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

	void SaveOptionsTreeState()
	{
		wchar_t str[128];
		TVITEMEX tvi;
		tvi.mask = TVIF_TEXT | TVIF_STATE;
		tvi.pszText = str;
		tvi.cchTextMax = _countof(str);
		tvi.hItem = m_pageTree.GetRoot();
		while (tvi.hItem != nullptr) {
			if (m_pageTree.GetItem(&tvi)) {
				char buf[130];
				mir_snprintf(buf, "%s%S", OPTSTATE_PREFIX, str);
				db_set_b(0, "Options", buf, (BYTE)((tvi.state & TVIS_EXPANDED) ? 1 : 0));
			}
			tvi.hItem = m_pageTree.GetNextSibling(tvi.hItem);
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
		m_keywordFilter.AddString(CORE_MODULES_FILTER, (LPARAM)g_hInst);

		for (int i = 0; i < m_arOpd.getCount(); i++) {
			OptionsPageData *opd = m_arOpd[i];
			opd->FindFilterStrings(false, 0, m_hwnd); // only modules name (fast enougth)

			HINSTANCE inst = opd->getInst();
			if (inst == g_hInst)
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

		opd->insideTab = IsInsideTab(m_currentPage);
		if (opd->insideTab) {
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
			GetModuleFileName(g_hInst, szFileName, _countof(szFileName));
			wchar_t *pos = wcsrchr(szFileName, '\\');
			if (pos)
				pos++;
			else
				pos = szFileName;

			wcsncpy_s(m_szFilterString, pos, _TRUNCATE);
		}
		else {
			int sel = m_keywordFilter.GetCurSel();
			if (sel != -1) {
				HINSTANCE hinst = (HINSTANCE)m_keywordFilter.GetItemData(sel);
				wchar_t szFileName[300];
				GetModuleFileName(hinst, szFileName, _countof(szFileName));
				wchar_t *pos = wcsrchr(szFileName, '\\');
				if (pos) pos++;
				else pos = szFileName;
				wcsncpy_s(m_szFilterString, pos, _TRUNCATE);
			}
		}

		_tcslwr_locale(m_szFilterString); //all strings are stored as lowercase ... make sure filter string is lowercase too

		m_pageTree.SendMsg(WM_SETREDRAW, FALSE, 0);

		HWND oldWnd = nullptr;
		HWND oldTab = nullptr;
		CMStringW fullTitle;
		TVITEMEX tvi;

		OptionsPageData *opd = getCurrent();
		if (opd != nullptr) {
			oldWnd = opd->getHwnd();
			if (opd->insideTab)
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
			wchar_t *ptszGroup = TranslateW_LP(opd->ptszGroup, opd->hLangpack);
			wchar_t *ptszTitle = opd->getString(opd->ptszTitle), *useTitle;
			wchar_t *ptszTab = TranslateW_LP(opd->ptszTab, opd->hLangpack);

			tvis.hParent = nullptr;
			useTitle = ptszTitle;

			if (ptszGroup != nullptr) {
				tvis.hParent = FindNamedTreeItem(nullptr, ptszGroup);
				if (tvis.hParent == nullptr) {
					tvis.item.lParam = -1;
					tvis.item.pszText = ptszGroup;
					tvis.hParent = m_pageTree.InsertItem(&tvis);
				}
			}
			else {
				tvi.hItem = FindNamedTreeItem(nullptr, useTitle);
				if (tvi.hItem != nullptr) {
					if (i == m_currentPage) m_hCurrentPage = tvi.hItem;
					tvi.mask = TVIF_PARAM;
					m_pageTree.GetItem(&tvi);
					if (tvi.lParam == -1) {
						tvi.lParam = i;
						m_pageTree.SetItem(&tvi);
						continue;
					}
				}
			}

			if (ptszTab != nullptr) {
				HTREEITEM hItem;
				if (tvis.hParent == nullptr)
					hItem = FindNamedTreeItem(nullptr, useTitle);
				else
					hItem = FindNamedTreeItem(tvis.hParent, useTitle);
				if (hItem != nullptr) {
					if (i == m_currentPage) {
						tvi.hItem = hItem;
						tvi.mask = TVIF_PARAM;
						tvi.lParam = m_currentPage;
						m_pageTree.SetItem(&tvi);
						m_hCurrentPage = hItem;
					}
					continue;
				}
			}

			tvis.item.pszText = useTitle;
			tvis.item.lParam = i;
			opd->hTreeItem = m_pageTree.InsertItem(&tvis);
			if (i == m_currentPage)
				m_hCurrentPage = opd->hTreeItem;
		}

		wchar_t str[128];
		tvi.mask = TVIF_TEXT | TVIF_STATE;
		tvi.pszText = str;
		tvi.cchTextMax = _countof(str);
		tvi.hItem = m_pageTree.GetRoot();
		while (tvi.hItem != nullptr) {
			if (m_pageTree.GetItem(&tvi)) {
				char buf[130];
				mir_snprintf(buf, "%s%S", OPTSTATE_PREFIX, str);
				if (!db_get_b(0, "Options", buf, 1))
					m_pageTree.Expand(tvi.hItem, TVE_COLLAPSE);
			}
			tvi.hItem = m_pageTree.GetNextSibling(tvi.hItem);
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
				if (oldTab && (opd == nullptr || !opd->insideTab))
					ShowWindow(oldTab, SW_HIDE);
			}
		}

		if (m_szFilterString[0] == 0) // Clear the keyword combo box
			m_keywordFilter.SetText(L"");
		if (!bRemoveFocusFromFilter)
			SetFocus(m_keywordFilter.GetHwnd()); //set the focus back to the combo box

		m_keywordFilter.SendMsg(CB_SETEDITSEL, 0, oldSel); //but don't select any of the text

		m_pageTree.SendMsg(WM_SETREDRAW, TRUE, 0);
		m_pageTree.EnsureVisible(m_hCurrentPage);
	}

	BOOL IsInsideTab(int i)
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

	void LoadOptionsModule(HINSTANCE hInst)
	{
		OptionsPageList arPages(1);
		CallPluginEventHook(hInst, hOptionsInitEvent, (WPARAM)&arPages, 0);
		if (arPages.getCount() == 0)
			return;

		for (int i = 0; i < arPages.getCount(); i++) {
			OptionsPageData *opd = new OptionsPageData(arPages[i]);
			if (opd->pDialog == nullptr) // smth went wrong
				delete opd;
			else
				m_arOpd.insert(opd);
		}

		RebuildPageTree();
	}

	void UnloadOptionsModule(HINSTANCE hInst)
	{
		bool bToRebuildTree = false;

		for (int i = m_arOpd.getCount() - 1; i >= 0; i--) {
			OptionsPageData *opd = m_arOpd[i];
			if (opd->getInst() != hInst)
				continue;

			if (m_currentPage > i)
				m_currentPage--;

			m_arOpd.remove(i);
			delete opd;
			bToRebuildTree = true;
		}

		if (bToRebuildTree)
			RebuildPageTree();
	}

	OptionsPageData* getCurrent() const
	{	return (m_currentPage == -1) ? nullptr : m_arOpd[m_currentPage];
	}

public:
	COptionsDlg(const wchar_t *pszCaption, const wchar_t *pszGroup, const wchar_t *pszPage, const wchar_t *pszTab, bool bSinglePage, const OptionsPageList &arPages) :
		CDlgBase(g_hInst, bSinglePage ? IDD_OPTIONSPAGE : IDD_OPTIONS),
		m_btnApply(this, IDC_APPLY),
		m_btnCancel(this, IDCANCEL),
		m_pageTree(this, IDC_PAGETREE),
		m_keywordFilter(this, IDC_KEYWORD_FILTER),
		m_arOpd(10),
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
	}

	virtual void OnInitDialog() override
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

		m_hPluginLoad = HookEventMessage(ME_SYSTEM_MODULELOAD, m_hwnd, HM_MODULELOAD);
		m_hPluginUnload = HookEventMessage(ME_SYSTEM_MODULEUNLOAD, m_hwnd, HM_MODULEUNLOAD);
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

		for (int i = 0; i < m_pages.getCount(); i++) {
			const OPTIONSDIALOGPAGE &odp = m_pages[i];
			OptionsPageData *opd = new OptionsPageData(odp);
			if (opd->pDialog == nullptr) // smth went wrong
				delete opd;
			else
				m_arOpd.insert(opd);

			if (!mir_wstrcmp(lastPage, odp.szTitle.w) && !mir_wstrcmp(lastGroup, odp.szGroup.w))
				if ((m_szTab == nullptr && m_currentPage == -1) || !mir_wstrcmp(lastTab, odp.szTab.w))
					m_currentPage = (int)i;
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
	}

	virtual void OnDestroy() override
	{
		ClearFilterStrings();
		m_szFilterString[0] = 0;

		UnhookEvent(m_hPluginLoad);
		UnhookEvent(m_hPluginUnload);

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

		for (int i = 0; i < m_arOpd.getCount(); i++)
			delete m_arOpd[i];

		DeleteObject(m_hBoldFont);
		pOptionsDlg = nullptr;
	}

	virtual void OnApply() override
	{
		PSHNOTIFY pshn;
		m_btnApply.Disable();
		SetFocus(m_pageTree.GetHwnd());

		OptionsPageData *opd = getCurrent();
		if (opd != nullptr) {
			pshn.hdr.idFrom = 0;
			pshn.lParam = IDC_APPLY;
			pshn.hdr.code = PSN_KILLACTIVE;
			pshn.hdr.hwndFrom = opd->getHwnd();
			if (SendMessage(opd->getHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn))
				return;
		}

		pshn.hdr.code = PSN_APPLY;
		for (int i = 0; i < m_arOpd.getCount(); i++) {
			OptionsPageData *p = m_arOpd[i];
			if (p->getHwnd() == nullptr || !p->changed) continue;
			p->changed = 0;
			pshn.hdr.hwndFrom = p->getHwnd();
			if (SendMessage(p->getHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn) == PSNRET_INVALID_NOCHANGEPAGE) {
				m_hCurrentPage = p->hTreeItem;
				m_pageTree.SelectItem(m_hCurrentPage);
				if (opd)
					opd->pDialog->Hide();
				m_currentPage = i;
				if (opd)
					opd->pDialog->Show();
				return;
			}
		}
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
		for (int i = 0; i < m_arOpd.getCount(); i++) {
			OptionsPageData *p = m_arOpd[i];
			if (p->getHwnd() == nullptr || !p->changed)
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
		// add a timer - when the timer elapses filter the option pages
		CTimer *pTimer = new CTimer(this, FILTER_TIMEOUT_TIMER);
		pTimer->OnEvent = Callback(this, &COptionsDlg::OnTimer);
		pTimer->Start(400);
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

		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		m_pageTree.GetItem(&tvi);
		m_currentPage = tvi.lParam;
		ShowWindow(GetDlgItem(m_hwnd, IDC_TAB), SW_HIDE);

		opd = getCurrent();
		if (opd == nullptr) {
			ShowWindow(GetDlgItem(m_hwnd, IDC_STNOPAGE), SW_SHOW);
			return;
		}

		if (opd->getHwnd() == nullptr)
			CreateOptionWindowEx(opd);

		opd->insideTab = IsInsideTab(m_currentPage);
		if (opd->insideTab) {
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

				tie.pszText = TranslateW_LP(p->ptszTab, p->hLangpack);
				tie.lParam = i;
				TabCtrl_InsertItem(hwndTab, pages, &tie);
				if (!mir_wstrcmp(opd->ptszTab, p->ptszTab))
					sel = pages;
				pages++;
			}
			TabCtrl_SetCurSel(hwndTab, sel);
			ShowWindow(hwndTab, opd->insideTab ? SW_SHOW : SW_HIDE);
		}

		::ThemeDialogBackground(opd->getHwnd(), opd->insideTab);

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

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case HM_MODULELOAD:
			LoadOptionsModule((HINSTANCE)lParam);
			break;

		case HM_MODULEUNLOAD:
			UnloadOptionsModule((HINSTANCE)lParam);
			break;

		case PSM_CHANGED:
			m_btnApply.Enable();
			{
				OptionsPageData *opd = getCurrent();
				if (opd)
					opd->changed = 1;
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

	void OnTimer(CTimer *pTimer)
	{
		pTimer->Stop();
		SaveOptionsTreeState();
		RebuildPageTree();
	}

	void Locate(const wchar_t *pszGroup, const wchar_t *pszPage, int _hLang)
	{
		ShowWindow(GetHwnd(), SW_RESTORE);
		SetForegroundWindow(m_hwnd);
		if (pszPage != nullptr) {
			HTREEITEM hItem = nullptr;
			if (pszGroup != nullptr) {
				hItem = FindNamedTreeItem(nullptr, TranslateW_LP(pszGroup, _hLang));
				if (hItem != nullptr)
					hItem = FindNamedTreeItem(hItem, TranslateW_LP(pszPage, _hLang));
			}
			else hItem = FindNamedTreeItem(nullptr, TranslateW_LP(pszPage, _hLang));

			if (hItem != nullptr)
				m_pageTree.SelectItem(hItem);
		}
	}
};

void OpenAccountOptions(PROTOACCOUNT *pa)
{
	if (pa->ppro == nullptr)
		return;

	OptionsPageList arPages(1);
	pa->ppro->OnEvent(EV_PROTO_ONOPTIONS, (WPARAM)&arPages, 0);
	if (arPages.getCount() == 0)
		return;

	wchar_t tszTitle[100];
	mir_snwprintf(tszTitle, TranslateT("%s options"), pa->tszAccountName);
	pOptionsDlg = new COptionsDlg(tszTitle, LPGENW("Network"), pa->tszAccountName, nullptr, true, arPages);
	pOptionsDlg->Show();
}

static void OpenOptionsNow(int _hLang, const wchar_t *pszGroup, const wchar_t *pszPage, const wchar_t *pszTab, bool bSinglePage)
{
	if (pOptionsDlg == nullptr) {
		OptionsPageList arPages(1);
		NotifyEventHooks(hOptionsInitEvent, (WPARAM)&arPages, 0);
		if (arPages.getCount() == 0)
			return;

		pOptionsDlg = new COptionsDlg(TranslateT("Miranda NG options"), pszGroup, pszPage, pszTab, bSinglePage, arPages);
		pOptionsDlg->Show();
	}
	else pOptionsDlg->Locate(pszGroup, pszPage, _hLang);
}

MIR_APP_DLL(int) Options_Open(const wchar_t *pszGroup, const wchar_t *pszPage, const wchar_t *pszTab, int _hLangpack)
{
	OpenOptionsNow(_hLangpack, pszGroup, pszPage, pszTab, false);
	return 0;
}

MIR_APP_DLL(HWND) Options_OpenPage(const wchar_t *pszGroup, const wchar_t *pszPage, const wchar_t *pszTab, int _hLangpack)
{
	OpenOptionsNow(_hLangpack, pszGroup, pszPage, pszTab, true);
	return pOptionsDlg->GetHwnd();
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Options_AddPage(WPARAM wParam, OPTIONSDIALOGPAGE *odp, int _hLangpack)
{
	OptionsPageList *pList = (OptionsPageList*)wParam;
	if (odp == nullptr || pList == nullptr)
		return 1;

	OptionsPage *dst = new OptionsPage();
	memcpy(dst, odp, sizeof(OPTIONSDIALOGPAGE));
	dst->hLangpack = _hLangpack;

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

	pList->insert(dst);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR OpenOptionsDialog(WPARAM, LPARAM)
{
	OpenOptionsNow(0, nullptr, nullptr, nullptr, false);
	return 0;
}

static int OptModulesLoaded(WPARAM, LPARAM)
{
	CMenuItem mi;
	SET_UID(mi, 0xc1284523, 0x548d, 0x4744, 0xb0, 0x9, 0xfb, 0xa0, 0x4, 0x8e, 0xa8, 0x67);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	mi.position = 1900000000;
	mi.name.a = LPGEN("&Options...");
	mi.pszService = "Options/OptionsCommand";
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, OpenOptionsDialog);
	return 0;
}

int ShutdownOptionsModule(WPARAM, LPARAM)
{
	delete pOptionsDlg; pOptionsDlg = nullptr;
	return 0;
}

int LoadOptionsModule(void)
{
	hOptionsInitEvent = CreateHookableEvent(ME_OPT_INITIALISE);
	HookEvent(ME_OPT_INITIALISE, LangpackOptionsInit);

	HookEvent(ME_SYSTEM_MODULESLOADED, OptModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownOptionsModule);
	return 0;
}
