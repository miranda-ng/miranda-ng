/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include "filter.h"

#define FILTER_TIMEOUT_TIMER 10012

#define ALL_MODULES_FILTER LPGEN("<all modules>")
#define CORE_MODULES_FILTER LPGEN("<core modules>")

int LangpackOptionsInit(WPARAM, LPARAM);

static HANDLE hOptionsInitEvent;
static HWND hwndOptions = NULL;
static HWND hFilterSearchWnd = NULL;

// Thread for search keywords in dialogs
static BYTE bSearchState = 0; // 0 - not executed; 1 - in progress; 2 - completed;
static int FilterPage = 0;
static int FilterLoadProgress = 100;
static int FilterTimerId = 0;

struct OptionsPageInit
{
	int pageCount;
	OPTIONSDIALOGPAGE *odp;
};

/////////////////////////////////////////////////////////////////////////////////////////

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

		if (msg == WM_DESTROY)
			m_hwnd = NULL;

		return res;
	}
};

struct OptionsPageData : public MZeroedObject
{
	OptionsPageData(OPTIONSDIALOGPAGE *src)
	{
		if (src->hInstance != NULL && src->pszTemplate != NULL)
			pDialog = new COptionPageDialog(src->hInstance, (int)src->pszTemplate, src->pfnDlgProc, src->dwInitParam);
		else
			pDialog = src->pDialog;

		flags = src->flags;
		hLangpack = src->hLangpack;

		if (src->flags & ODPF_UNICODE)
			ptszTitle = mir_tstrdup(src->ptszTitle);
		else
			ptszTitle = mir_a2t(src->pszTitle);

		if (src->flags & ODPF_UNICODE)
			ptszGroup = mir_tstrdup(src->ptszGroup);
		else
			ptszGroup = mir_a2t(src->pszGroup);

		if (src->flags & ODPF_UNICODE)
			ptszTab = mir_tstrdup(src->ptszTab);
		else
			ptszTab = mir_a2t(src->pszTab);
  	}
	
	~OptionsPageData()
	{
		if (getHwnd() != NULL)
			DestroyWindow(getHwnd());
	}

	CDlgBase *pDialog;
	int hLangpack;
	ptrT ptszTitle, ptszGroup, ptszTab;
	HTREEITEM hTreeItem;
	int changed;
	int height;
	int width;
	DWORD flags;
	BOOL insideTab;

	__forceinline HWND getHwnd() const { return pDialog->GetHwnd(); }
	__forceinline HINSTANCE getInst() const { return pDialog->GetInst(); }

	__forceinline TCHAR* getString(TCHAR *ptszStr)
	{
		if (flags & ODPF_DONTTRANSLATE)
			return ptszStr;
		return TranslateTH(hLangpack, ptszStr);
	}
};

struct OptionsDlgData : public MZeroedObject
{
	OptionsDlgData() :
		arOpd(10)
		{}

	int currentPage;
	HTREEITEM hCurrentPage;
	LIST<OptionsPageData> arOpd;
	RECT rcDisplay;
	RECT rcTab;
	HFONT hBoldFont;
	TCHAR szFilterString[1024];
	HANDLE hPluginLoad, hPluginUnload;

	OptionsPageData* getCurrent() const
	{	return (currentPage == -1) ? NULL : arOpd[currentPage];
	}
};

HTREEITEM FindNamedTreeItemAtRoot(HWND hwndTree, const TCHAR* name)
{
	TCHAR str[128];
	TVITEM tvi;
	tvi.mask = TVIF_TEXT;
	tvi.pszText = str;
	tvi.cchTextMax = SIZEOF(str);
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem != NULL) {
		SendMessage(hwndTree, TVM_GETITEM, 0, (LPARAM)&tvi);
		if (!_tcsicmp(str, name))
			return tvi.hItem;

		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return NULL;
}

static HTREEITEM FindNamedTreeItemAtChildren(HWND hwndTree, HTREEITEM hItem, const TCHAR* name)
{
	TCHAR str[128];
	TVITEM tvi;
	tvi.mask = TVIF_TEXT;
	tvi.pszText = str;
	tvi.cchTextMax = SIZEOF(str);
	tvi.hItem = TreeView_GetChild(hwndTree, hItem);
	while (tvi.hItem != NULL) {
		SendMessage(hwndTree, TVM_GETITEM, 0, (LPARAM)&tvi);
		if (!_tcsicmp(str, name))
			return tvi.hItem;

		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return NULL;
}

static BOOL CALLBACK BoldGroupTitlesEnumChildren(HWND hwnd, LPARAM lParam)
{
	TCHAR szClass[64];
	GetClassName(hwnd, szClass, SIZEOF(szClass));

	if (!mir_tstrcmp(szClass, _T("Button")) && (GetWindowLongPtr(hwnd, GWL_STYLE) & 0x0F) == BS_GROUPBOX)
		SendMessage(hwnd, WM_SETFONT, lParam, 0);
	return TRUE;
}

#define OPTSTATE_PREFIX "s_"

static void SaveOptionsTreeState(HWND hdlg)
{
	TVITEMA tvi;
	char buf[130], str[128];
	tvi.mask = TVIF_TEXT | TVIF_STATE;
	tvi.pszText = str;
	tvi.cchTextMax = SIZEOF(str);
	tvi.hItem = TreeView_GetRoot(GetDlgItem(hdlg, IDC_PAGETREE));
	while (tvi.hItem != NULL) {
		if (SendDlgItemMessageA(hdlg, IDC_PAGETREE, TVM_GETITEMA, 0, (LPARAM)&tvi)) {
			mir_snprintf(buf, SIZEOF(buf), "%s%s", OPTSTATE_PREFIX, str);
			db_set_b(NULL, "Options", buf, (BYTE)((tvi.state & TVIS_EXPANDED) ? 1 : 0));
		}
		tvi.hItem = TreeView_GetNextSibling(GetDlgItem(hdlg, IDC_PAGETREE), tvi.hItem);
	}
}

#define DM_FOCUSPAGE   (WM_USER+10)
#define DM_REBUILDPAGETREE (WM_USER+11)

#define HM_MODULELOAD (WM_USER+12)
#define HM_MODULEUNLOAD (WM_USER+13)

static void ThemeDialogBackground(HWND hwnd, BOOL tabbed)
{
	EnableThemeDialogTexture(hwnd, (tabbed ? ETDT_ENABLE : ETDT_DISABLE) | ETDT_USETABTEXTURE);
}

static TCHAR* GetPluginName(HINSTANCE hInstance, TCHAR *buffer, int size)
{
	TCHAR tszModuleName[MAX_PATH];
	GetModuleFileName(hInstance, tszModuleName, SIZEOF(tszModuleName));
	TCHAR *dllName = _tcsrchr(tszModuleName, '\\');
	if (!dllName)
		dllName = tszModuleName;
	else
		dllName++;

	_tcsncpy_s(buffer, size, dllName, _TRUNCATE);
	return buffer;
}

PageHash GetPluginPageHash(const OptionsPageData *page)
{
	return mir_hashstrT(page->ptszGroup) + mir_hashstrT(page->ptszTitle) + mir_hashstrT(page->ptszTab);
}

static HWND CreateOptionWindow(const OptionsPageData *opd, HWND hWndParent)
{
	opd->pDialog->SetParent(hWndParent);
	opd->pDialog->Create();
	return opd->pDialog->GetHwnd();
}

static void FindFilterStrings(int enableKeywordFiltering, int current, HWND hWndParent, const OptionsPageData *page)
{
	HWND hWnd = 0;
	if (enableKeywordFiltering) {
		if (current)
			hWnd = page->getHwnd();
		else {
			hWnd = CreateOptionWindow(page, hWndParent);
			ShowWindow(hWnd, SW_HIDE); // make sure it's hidden
		}
	}

	DWORD key = GetPluginPageHash(page); // get the plugin page hash

	TCHAR pluginName[MAX_PATH];
	char *temp = GetPluginNameByInstance(page->getInst());
	GetDialogStrings(enableKeywordFiltering, key, GetPluginName(page->getInst(), pluginName, SIZEOF(pluginName)), hWnd, page->ptszGroup, page->ptszTitle, page->ptszTab, _A2T(temp));

	if (enableKeywordFiltering && !current)
		DestroyWindow(hWnd); // destroy the page, we're done with it
}

static int MatchesFilter(const OptionsPageData *page, TCHAR *szFilterString)
{
	return ContainsFilterString(GetPluginPageHash(page), szFilterString);
}

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

	TCHAR buf[255];
	if (bSearchState == 1 && FilterLoadProgress < 100 && FilterLoadProgress > 0)
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("Loading... %d%%"), FilterLoadProgress);
	else
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("Search"));

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

		wchar_t *bufW = mir_t2u(buf);
		DrawThemeText(hTheme, hdc, EP_EDITTEXT, ETS_DISABLED, bufW, -1, 0, 0, &rc);
		mir_free(bufW);

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
		int oldMode = SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, buf, -1, &rc, 0);
		SetBkMode(hdc, oldMode);
		SelectObject(hdc, oldFont);
	}

	if (message == WM_PAINT)
		EndPaint(hWnd, &paint);

	return 0;
}

static bool CheckPageShow(HWND hdlg, OptionsDlgData *dat, int i)
{
	OptionsPageData *opd = dat->arOpd[i];
	if (dat->szFilterString[0] && !MatchesFilter(opd, dat->szFilterString))
		return false;
	return true;
}

static BOOL IsAeroMode()
{
	BOOL result;
	return dwmIsCompositionEnabled && (dwmIsCompositionEnabled(&result) == S_OK) && result;
}

static void FreeOptionsData(OptionsPageInit* popi)
{
	for (int i = 0; i < popi->pageCount; i++) {
		mir_free((char*)popi->odp[i].pszTitle);
		mir_free(popi->odp[i].pszGroup);
		mir_free(popi->odp[i].pszTab);
		if ((DWORD_PTR)popi->odp[i].pszTemplate & 0xFFFF0000)
			mir_free((char*)popi->odp[i].pszTemplate);
	}
	mir_free(popi->odp);
}

static LRESULT CALLBACK AeroPaintSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void AeroPaintControl(HWND hwnd, HDC hdc, UINT msg, LPARAM lpFlags)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	HDC tempDC = CreateCompatibleDC(hdc);

	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = rc.right;
	bmi.bmiHeader.biHeight = -rc.bottom;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	BYTE *pBits;
	HBITMAP hBmp = CreateDIBSection(tempDC, &bmi, DIB_RGB_COLORS, (void **)&pBits, NULL, 0);
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
			RedrawWindow((HWND)lParam, NULL, NULL, RDW_INVALIDATE);
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

static void CALLBACK FilterSearchTimerFunc(HWND hwnd, UINT, UINT_PTR, DWORD)
{
	OptionsDlgData *dat = (OptionsDlgData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!dat)
		return;

	if (hFilterSearchWnd == NULL)
		hFilterSearchWnd = CreateWindowA("STATIC", "Test", WS_OVERLAPPED | WS_DISABLED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), 0); // Fake window to keep option page focused

	if (FilterPage < dat->arOpd.getCount())
		FindFilterStrings(TRUE, dat->currentPage == FilterPage, hFilterSearchWnd, dat->arOpd[FilterPage]);

	FilterPage++;
	FilterLoadProgress = FilterPage * 100 / ((dat->arOpd.getCount()) ? dat->arOpd.getCount() : FilterPage);
	if (FilterPage >= dat->arOpd.getCount()) {
		KillTimer(hwnd, FilterTimerId);
		FilterTimerId = 0;
		bSearchState = 2;
		FilterLoadProgress = 100;
		DestroyWindow(hFilterSearchWnd);
		hFilterSearchWnd = NULL;
	}
	RedrawWindow(GetDlgItem(hwnd, IDC_KEYWORD_FILTER), NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
}

static void ExecuteFindFilterStringsTimer(HWND hdlg)
{
	bSearchState = 1;
	FilterPage = 0;
	if (FilterTimerId) KillTimer(hdlg, FilterTimerId);
	FilterTimerId = SetTimer(hdlg, NULL, 1, FilterSearchTimerFunc);
}

static void FillFilterCombo(HWND hDlg, OptionsDlgData* dat)
{
	HINSTANCE *KnownInstances = (HINSTANCE*)alloca(sizeof(HINSTANCE)*dat->arOpd.getCount());
	int countKnownInst = 0;
	SendDlgItemMessage(hDlg, IDC_KEYWORD_FILTER, (UINT)CB_RESETCONTENT, 0, 0);
	int index = SendDlgItemMessage(hDlg, IDC_KEYWORD_FILTER, (UINT)CB_ADDSTRING, 0, (LPARAM)TranslateT(ALL_MODULES_FILTER));
	SendDlgItemMessage(hDlg, IDC_KEYWORD_FILTER, (UINT)CB_SETITEMDATA, (WPARAM)index, 0);
	index = SendDlgItemMessage(hDlg, IDC_KEYWORD_FILTER, (UINT)CB_ADDSTRING, 0, (LPARAM)TranslateT(CORE_MODULES_FILTER));
	SendDlgItemMessage(hDlg, IDC_KEYWORD_FILTER, (UINT)CB_SETITEMDATA, (WPARAM)index, (LPARAM)hInst);
	
	for (int i = 0; i < dat->arOpd.getCount(); i++) {
		OptionsPageData *opd = dat->arOpd[i];
		FindFilterStrings(FALSE, FALSE, hDlg, opd); // only modules name (fast enougth)

		HINSTANCE inst = opd->getInst();
		if (inst == hInst)
			continue;

		int j;
		for (j = 0; j < countKnownInst; j++)
			if (KnownInstances[j] == inst)
				break;
		if (j != countKnownInst)
			continue;

		KnownInstances[countKnownInst] = inst;
		countKnownInst++;
		
		TCHAR tszModuleName[MAX_PATH];
		GetModuleFileName(inst, tszModuleName, SIZEOF(tszModuleName));

		TCHAR *dllName = mir_a2t(GetPluginNameByInstance(inst));
		if (!dllName) dllName = mir_tstrdup(_tcsrchr(tszModuleName, _T('\\')));
		if (!dllName) dllName = mir_tstrdup(tszModuleName);
		if (dllName) {
			index = SendDlgItemMessage(hDlg, IDC_KEYWORD_FILTER, (UINT)CB_ADDSTRING, 0, (LPARAM)dllName);
			SendDlgItemMessage(hDlg, IDC_KEYWORD_FILTER, (UINT)CB_SETITEMDATA, (WPARAM)index, (LPARAM)inst);
			mir_free(dllName);
		}
	}

	FilterLoadProgress = 100;
}

static void RebuildPageTree(HWND hdlg, OptionsDlgData *dat)
{
	LPARAM oldSel = SendDlgItemMessage(hdlg, IDC_KEYWORD_FILTER, CB_GETEDITSEL, 0, 0);
	GetDlgItemText(hdlg, IDC_KEYWORD_FILTER, dat->szFilterString, SIZEOF(dat->szFilterString));

	// if filter string is set to all modules then make the filter string empty (this will return all modules)
	BOOL bRemoveFocusFromFilter = FALSE;
	if (_tcscmp(dat->szFilterString, TranslateT(ALL_MODULES_FILTER)) == 0) {
		dat->szFilterString[0] = 0;
		bRemoveFocusFromFilter = TRUE;
	}
	// if filter string is set to core modules replace it with the name of the executable (this will return all core modules)
	else if (_tcscmp(dat->szFilterString, TranslateT(CORE_MODULES_FILTER)) == 0) {
		// replace string with process name - that will show core settings
		TCHAR szFileName[300];
		GetModuleFileName(NULL, szFileName, SIZEOF(szFileName));
		TCHAR *pos = _tcsrchr(szFileName, _T('\\'));
		if (pos)
			pos++;
		else
			pos = szFileName;

		_tcsncpy_s(dat->szFilterString, pos, _TRUNCATE);
	}
	else {
		int sel = SendDlgItemMessage(hdlg, IDC_KEYWORD_FILTER, (UINT)CB_GETCURSEL, 0, 0);
		if (sel != -1) {
			HINSTANCE hinst = (HINSTANCE)SendDlgItemMessage(hdlg, IDC_KEYWORD_FILTER, (UINT)CB_GETITEMDATA, sel, 0);
			TCHAR szFileName[300];
			GetModuleFileName(hinst, szFileName, SIZEOF(szFileName));
			TCHAR *pos = _tcsrchr(szFileName, _T('\\'));
			if (pos) pos++;
			else pos = szFileName;
			_tcsncpy_s(dat->szFilterString, pos, _TRUNCATE);
		}
	}

	_tcslwr_locale(dat->szFilterString); //all strings are stored as lowercase ... make sure filter string is lowercase too

	HWND hwndTree = GetDlgItem(hdlg, IDC_PAGETREE);
	SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0);

	HWND oldWnd = NULL;
	HWND oldTab = NULL;
	CMString fullTitle;

	OptionsPageData *opd = dat->getCurrent();
	if (opd != NULL) {
		oldWnd = opd->getHwnd();
		if (opd->insideTab)
			oldTab = GetDlgItem(hdlg, IDC_TAB);
	}

	dat->hCurrentPage = NULL;

	TreeView_SelectItem(hwndTree, NULL);
	TreeView_DeleteAllItems(hwndTree);

	TVINSERTSTRUCT tvis;
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_SORT;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
	tvis.item.state = tvis.item.stateMask = TVIS_EXPANDED;
	for (int i = 0; i < dat->arOpd.getCount(); i++) {
		if (!CheckPageShow(hdlg, dat, i))
			continue;

		opd = dat->arOpd[i];
		TCHAR *ptszGroup = TranslateTH(opd->hLangpack, opd->ptszGroup);
		TCHAR *ptszTitle = opd->getString(opd->ptszTitle), *useTitle;
		TCHAR *ptszTab = TranslateTH(opd->hLangpack, opd->ptszTab);

		tvis.hParent = NULL;
		useTitle = ptszTitle;

		if (ptszGroup != NULL) {
			tvis.hParent = FindNamedTreeItemAtRoot(hwndTree, ptszGroup);
			if (tvis.hParent == NULL) {
				tvis.item.lParam = -1;
				tvis.item.pszText = ptszGroup;
				tvis.hParent = TreeView_InsertItem(hwndTree, &tvis);
			}
		}
		else {
			TVITEM tvi;
			tvi.hItem = FindNamedTreeItemAtRoot(hwndTree, useTitle);
			if (tvi.hItem != NULL) {
				if (i == dat->currentPage) dat->hCurrentPage = tvi.hItem;
				tvi.mask = TVIF_PARAM;
				TreeView_GetItem(hwndTree, &tvi);
				if (tvi.lParam == -1) {
					tvi.lParam = i;
					TreeView_SetItem(hwndTree, &tvi);
					continue;
				}
			}
		}

		if (ptszTab != NULL) {
			HTREEITEM hItem;
			if (tvis.hParent == NULL)
				hItem = FindNamedTreeItemAtRoot(hwndTree, useTitle);
			else
				hItem = FindNamedTreeItemAtChildren(hwndTree, tvis.hParent, useTitle);
			if (hItem != NULL) {
				if (i == dat->currentPage) {
					TVITEM tvi;
					tvi.hItem = hItem;
					tvi.mask = TVIF_PARAM;
					tvi.lParam = dat->currentPage;
					TreeView_SetItem(hwndTree, &tvi);
					dat->hCurrentPage = hItem;
				}
				continue;
			}
		}

		tvis.item.pszText = useTitle;
		tvis.item.lParam = i;
		opd->hTreeItem = TreeView_InsertItem(hwndTree, &tvis);
		if (i == dat->currentPage)
			dat->hCurrentPage = opd->hTreeItem;
	}

	char str[128];
	TVITEMA tvi;
	tvi.mask = TVIF_TEXT | TVIF_STATE;
	tvi.pszText = str;
	tvi.cchTextMax = SIZEOF(str);
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem != NULL) {
		if (SendMessageA(hwndTree, TVM_GETITEMA, 0, (LPARAM)&tvi)) {
			char buf[130];
			mir_snprintf(buf, SIZEOF(buf), "%s%s", OPTSTATE_PREFIX, str);
			if (!db_get_b(NULL, "Options", buf, 1))
				TreeView_Expand(hwndTree, tvi.hItem, TVE_COLLAPSE);
		}
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}

	if (dat->hCurrentPage == NULL) {
		dat->hCurrentPage = TreeView_GetRoot(hwndTree);
		dat->currentPage = -1;
	}
	TreeView_SelectItem(hwndTree, dat->hCurrentPage);

	if (oldWnd) {
		opd = dat->getCurrent();
		if (opd && oldWnd != opd->getHwnd()) {
			ShowWindow(oldWnd, SW_HIDE);
			if (oldTab && (opd == NULL || !opd->insideTab))
				ShowWindow(oldTab, SW_HIDE);
		}
	}

	if (dat->szFilterString[0] == 0) // Clear the keyword combo box
		SetDlgItemText(hdlg, IDC_KEYWORD_FILTER, _T(""));
	if (!bRemoveFocusFromFilter)
		SetFocus(GetDlgItem(hdlg, IDC_KEYWORD_FILTER)); //set the focus back to the combo box

	SendDlgItemMessage(hdlg, IDC_KEYWORD_FILTER, CB_SETEDITSEL, 0, oldSel); //but don't select any of the text

	SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0);
	TreeView_EnsureVisible(hwndTree, dat->hCurrentPage);
}

static BOOL IsInsideTab(HWND hdlg, OptionsDlgData *dat, int i)
{
	OptionsPageData *opd = dat->arOpd[i];
	int pages = 0;
	if (opd->ptszTab != NULL) {
		// Count tabs to calc position
		for (int j = 0; j < dat->arOpd.getCount() && pages < 2; j++) {
			OptionsPageData* opd2 = dat->arOpd[j];
			if (!CheckPageShow(hdlg, dat, j)) continue;
			if (mir_tstrcmp(opd2->ptszTitle, opd->ptszTitle) || mir_tstrcmp(opd2->ptszGroup, opd->ptszGroup))
				continue;
			pages++;
		}
	}
	return (pages > 1);
}

static void LoadOptionsModule(HWND hdlg, OptionsDlgData *dat, HINSTANCE hInst)
{
	OptionsPageInit opi = { 0 };
	CallPluginEventHook(hInst, hOptionsInitEvent, (WPARAM)&opi, 0);
	if (opi.pageCount == 0)
		return;

	for (int i = 0; i < opi.pageCount; i++)
		dat->arOpd.insert(new OptionsPageData(&opi.odp[i]));

	FreeOptionsData(&opi);
	PostMessage(hdlg, DM_REBUILDPAGETREE, 0, 0);
}

static void UnloadOptionsModule(HWND hdlg, OptionsDlgData *dat, HINSTANCE hInst)
{
	bool bToRebuildTree = false;

	for (int i = dat->arOpd.getCount() - 1; i >= 0; i--) {
		OptionsPageData *opd = dat->arOpd[i];
		if (opd->getInst() != hInst)
			continue;

		if (dat->currentPage > i)
			dat->currentPage--;

		dat->arOpd.remove(i);
		delete opd;
		bToRebuildTree = true;
	}

	if (bToRebuildTree)
		PostMessage(hdlg, DM_REBUILDPAGETREE, 0, 0);
}

static INT_PTR CALLBACK OptionsDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OptionsPageData *opd;
	OptionsDlgData *dat = (OptionsDlgData*)GetWindowLongPtr(hdlg, GWLP_USERDATA);
	HWND hwndTree = GetDlgItem(hdlg, IDC_PAGETREE);

	switch (message) {
	case WM_CTLCOLORSTATIC:
		switch (GetDlgCtrlID((HWND)lParam)) {
		case IDC_WHITERECT:
		case IDC_KEYWORD_FILTER:
			SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
			return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
		}
		break;

	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);

		if (!ServiceExists(MS_MODERNOPT_SHOW))
			ShowWindow(GetDlgItem(hdlg, IDC_MODERN), FALSE);

		dat = new OptionsDlgData;
		SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)dat);

		Utils_RestoreWindowPositionNoSize(hdlg, NULL, "Options", "");
		Window_SetIcon_IcoLib(hdlg, SKINICON_OTHER_OPTIONS);
		EnableWindow(GetDlgItem(hdlg, IDC_APPLY), FALSE);
		{
			COMBOBOXINFO cbi;
			cbi.cbSize = sizeof(COMBOBOXINFO);
			GetComboBoxInfo(GetDlgItem(hdlg, IDC_KEYWORD_FILTER), &cbi);
			mir_subclassWindow(cbi.hwndItem, OptionsFilterSubclassProc);

			if (IsAeroMode()) {
				mir_subclassWindow(cbi.hwndCombo, AeroPaintSubclassProc);
				mir_subclassWindow(cbi.hwndItem, AeroPaintSubclassProc);
			}

			PROPSHEETHEADER *psh = (PROPSHEETHEADER*)lParam;
			SetWindowText(hdlg, psh->pszCaption);

			LOGFONT lf;
			dat->hBoldFont = (HFONT)SendDlgItemMessage(hdlg, IDC_APPLY, WM_GETFONT, 0, 0);
			GetObject(dat->hBoldFont, sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			dat->hBoldFont = CreateFontIndirect(&lf);

			dat->hPluginLoad = HookEventMessage(ME_SYSTEM_MODULELOAD, hdlg, HM_MODULELOAD);
			dat->hPluginUnload = HookEventMessage(ME_SYSTEM_MODULEUNLOAD, hdlg, HM_MODULEUNLOAD);
			dat->currentPage = -1;

			ptrT lastPage, lastGroup, lastTab;
			OPENOPTIONSDIALOG *ood = (OPENOPTIONSDIALOG*)psh->pStartPage;
			if (ood->pszPage == NULL) {
				lastPage = db_get_tsa(NULL, "Options", "LastPage");

				if (ood->pszGroup == NULL)
					lastGroup = db_get_tsa(NULL, "Options", "LastGroup");
				else
					lastGroup = mir_a2t(ood->pszGroup);
			}
			else {
				lastPage = mir_a2t(ood->pszPage);
				lastGroup = mir_a2t(ood->pszGroup);
			}

			if (ood->pszTab == NULL)
				lastTab = db_get_tsa(NULL, "Options", "LastTab");
			else
				lastTab = mir_a2t(ood->pszTab);

			OPTIONSDIALOGPAGE *odp = (OPTIONSDIALOGPAGE*)psh->ppsp;
			for (UINT i = 0; i < psh->nPages; i++, odp++) {
				dat->arOpd.insert(new OptionsPageData(odp));

				if (!mir_tstrcmp(lastPage, odp->ptszTitle) && !mir_tstrcmp(lastGroup, odp->ptszGroup))
					if ((ood->pszTab == NULL && dat->currentPage == -1) || !mir_tstrcmp(lastTab, odp->ptszTab))
						dat->currentPage = (int)i;
			}

			GetWindowRect(GetDlgItem(hdlg, IDC_STNOPAGE), &dat->rcDisplay);
			MapWindowPoints(NULL, hdlg, (LPPOINT)&dat->rcDisplay, 2);

			// Add an item to count in height
			TCITEM tie;
			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage = -1;
			tie.pszText = _T("X");
			TabCtrl_InsertItem(GetDlgItem(hdlg, IDC_TAB), 0, &tie);

			GetWindowRect(GetDlgItem(hdlg, IDC_TAB), &dat->rcTab);
			MapWindowPoints(NULL, hdlg, (LPPOINT)&dat->rcTab, 2);
			TabCtrl_AdjustRect(GetDlgItem(hdlg, IDC_TAB), FALSE, &dat->rcTab);

			FillFilterCombo(hdlg, dat);
			PostMessage(hdlg, DM_REBUILDPAGETREE, 0, 0);
		}
		return TRUE;

	case DM_REBUILDPAGETREE:
		RebuildPageTree(hdlg, dat);
		break;

	case HM_MODULELOAD:
		LoadOptionsModule(hdlg, dat, (HINSTANCE)lParam);
		break;

	case HM_MODULEUNLOAD:
		UnloadOptionsModule(hdlg, dat, (HINSTANCE)lParam);
		break;

	case PSM_CHANGED:
		EnableWindow(GetDlgItem(hdlg, IDC_APPLY), TRUE);

		opd = dat->getCurrent();
		if (opd)
			opd->changed = 1;

		return TRUE;

	case PSM_GETBOLDFONT:
		SetWindowLongPtr(hdlg, DWLP_MSGRESULT, (LONG_PTR)dat->hBoldFont);
		return TRUE;

	case WM_NOTIFY:
		switch (wParam) {
		case IDC_TAB:
		case IDC_PAGETREE:
			switch (((LPNMHDR)lParam)->code) {
			case TVN_ITEMEXPANDING:
				SetWindowLongPtr(hdlg, DWLP_MSGRESULT, FALSE);
				return TRUE;

			case TCN_SELCHANGING:
			case TVN_SELCHANGING:
				opd = dat->getCurrent();
				if (opd && opd->getHwnd() != NULL) {
					PSHNOTIFY pshn;
					pshn.hdr.code = PSN_KILLACTIVE;
					pshn.hdr.hwndFrom = dat->arOpd[dat->currentPage]->getHwnd();
					pshn.hdr.idFrom = 0;
					pshn.lParam = 0;
					if (SendMessage(dat->arOpd[dat->currentPage]->getHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn)) {
						SetWindowLongPtr(hdlg, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
				}
				break;

			case TCN_SELCHANGE:
			case TVN_SELCHANGED:
				ShowWindow(GetDlgItem(hdlg, IDC_STNOPAGE), SW_HIDE);

				opd = dat->getCurrent();
				if (opd && opd->getHwnd() != NULL)
					ShowWindow(opd->getHwnd(), SW_HIDE);

				if (wParam != IDC_TAB) {
					TVITEM tvi;
					tvi.hItem = dat->hCurrentPage = TreeView_GetSelection(hwndTree);
					if (tvi.hItem == NULL) {
						ShowWindow(GetDlgItem(hdlg, IDC_TAB), SW_HIDE);
						break;
					}

					tvi.mask = TVIF_HANDLE | TVIF_PARAM;
					TreeView_GetItem(hwndTree, &tvi);
					dat->currentPage = tvi.lParam;
					ShowWindow(GetDlgItem(hdlg, IDC_TAB), SW_HIDE);
				}
				else {
					TCITEM tie;
					tie.mask = TCIF_PARAM;
					TabCtrl_GetItem(GetDlgItem(hdlg, IDC_TAB), TabCtrl_GetCurSel(GetDlgItem(hdlg, IDC_TAB)), &tie);
					dat->currentPage = tie.lParam;

					TVITEM tvi;
					tvi.hItem = dat->hCurrentPage;
					tvi.mask = TVIF_PARAM;
					tvi.lParam = dat->currentPage;
					TreeView_SetItem(hwndTree, &tvi);
				}

				opd = dat->getCurrent();
				if (opd == NULL) {
					ShowWindow(GetDlgItem(hdlg, IDC_STNOPAGE), SW_SHOW);
					break;
				}
				if (opd->getHwnd() == NULL) {
					CreateOptionWindow(opd, hdlg);
					if (opd->flags & ODPF_BOLDGROUPS)
						EnumChildWindows(opd->getHwnd(), BoldGroupTitlesEnumChildren, (LPARAM)dat->hBoldFont);

					RECT rcPage;
					GetClientRect(opd->getHwnd(), &rcPage);
					int w = opd->width = rcPage.right;
					int h = opd->height = rcPage.bottom;

					RECT rc;
					GetWindowRect(opd->getHwnd(), &rc);

					opd->insideTab = IsInsideTab(hdlg, dat, dat->currentPage);
					if (opd->insideTab) {
						SetWindowPos(opd->getHwnd(), HWND_TOP, (dat->rcTab.left + dat->rcTab.right - w) >> 1, dat->rcTab.top, w, h, 0);
						ThemeDialogBackground(opd->getHwnd(), TRUE);
					}
					else {
						SetWindowPos(opd->getHwnd(), HWND_TOP, (dat->rcDisplay.left + dat->rcDisplay.right - w) >> 1, (dat->rcDisplay.top + dat->rcDisplay.bottom - h) >> 1, w, h, 0);
						ThemeDialogBackground(opd->getHwnd(), FALSE);
					}
				}

				if (wParam != IDC_TAB) {
					opd->insideTab = IsInsideTab(hdlg, dat, dat->currentPage);
					if (opd->insideTab) {
						// Make tabbed pane
						int pages = 0, sel = 0;
						HWND hwndTab = GetDlgItem(hdlg, IDC_TAB);
						TabCtrl_DeleteAllItems(hwndTab);

						TCITEM tie;
						tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
						tie.iImage = -1;
						for (int i = 0; i < dat->arOpd.getCount(); i++) {
							if (!CheckPageShow(hdlg, dat, i))
								continue;

							OptionsPageData *p = dat->arOpd[i];
							if (mir_tstrcmp(opd->ptszTitle, p->ptszTitle) || mir_tstrcmp(opd->ptszGroup, p->ptszGroup))
								continue;

							tie.pszText = TranslateTH(p->hLangpack, p->ptszTab);
							tie.lParam = i;
							TabCtrl_InsertItem(hwndTab, pages, &tie);
							if (!mir_tstrcmp(opd->ptszTab, p->ptszTab))
								sel = pages;
							pages++;
						}
						TabCtrl_SetCurSel(hwndTab, sel);
						ShowWindow(hwndTab, opd->insideTab ? SW_SHOW : SW_HIDE);
					}

					ThemeDialogBackground(opd->getHwnd(), opd->insideTab);
				}

				ShowWindow(opd->getHwnd(), SW_SHOW);
				if (((LPNMTREEVIEW)lParam)->action == TVC_BYMOUSE)
					PostMessage(hdlg, DM_FOCUSPAGE, 0, 0);
				else
					SetFocus(hwndTree);
			}
		}
		break;

	case DM_FOCUSPAGE:
		if (dat->currentPage != -1)
			SetFocus(dat->arOpd[dat->currentPage]->getHwnd());
		break;

	case WM_TIMER:
		if (wParam == FILTER_TIMEOUT_TIMER) {
			SaveOptionsTreeState(hdlg);
			SendMessage(hdlg, DM_REBUILDPAGETREE, 0, 0);

			KillTimer(hdlg, FILTER_TIMEOUT_TIMER);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_KEYWORD_FILTER:
			// add a timer - when the timer elapses filter the option pages
			if ((HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == CBN_EDITCHANGE))
			if (!SetTimer(hdlg, FILTER_TIMEOUT_TIMER, 400, NULL))
				MessageBeep(MB_ICONSTOP);
			break;

		case IDC_MODERN:
			db_set_b(NULL, "Options", "Expert", 0);
			SaveOptionsTreeState(hdlg);
			PostMessage(hdlg, WM_CLOSE, 0, 0);
			CallService(MS_MODERNOPT_SHOW, 0, 0);
			break;

		case IDCANCEL:
			{
				PSHNOTIFY pshn;
				pshn.hdr.idFrom = 0;
				pshn.lParam = 0;
				pshn.hdr.code = PSN_RESET;
				for (int i = 0; i < dat->arOpd.getCount(); i++) {
					OptionsPageData *p = dat->arOpd[i];
					if (p->getHwnd() == NULL || !p->changed)
						continue;
					pshn.hdr.hwndFrom = p->getHwnd();
					SendMessage(p->getHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn);
				}
				DestroyWindow(hdlg);
			}
			break;

		case IDOK:
		case IDC_APPLY:
			if (LOWORD(wParam) == IDOK && GetParent(GetFocus()) == GetDlgItem(hdlg, IDC_KEYWORD_FILTER))
				return TRUE;

			PSHNOTIFY pshn;
			EnableWindow(GetDlgItem(hdlg, IDC_APPLY), FALSE);
			SetFocus(hwndTree);

			opd = dat->getCurrent();
			if (opd != NULL) {
				pshn.hdr.idFrom = 0;
				pshn.lParam = LOWORD(wParam);
				pshn.hdr.code = PSN_KILLACTIVE;
				pshn.hdr.hwndFrom = opd->getHwnd();
				if (SendMessage(opd->getHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn))
					break;
			}

			pshn.hdr.code = PSN_APPLY;
			for (int i = 0; i < dat->arOpd.getCount(); i++) {
				OptionsPageData *p = dat->arOpd[i];
				if (p->getHwnd() == NULL || !p->changed) continue;
				p->changed = 0;
				pshn.hdr.hwndFrom = p->getHwnd();
				if (SendMessage(p->getHwnd(), WM_NOTIFY, 0, (LPARAM)&pshn) == PSNRET_INVALID_NOCHANGEPAGE) {
					dat->hCurrentPage = p->hTreeItem;
					TreeView_SelectItem(hwndTree, dat->hCurrentPage);
					if (opd)
						opd->pDialog->Show(SW_HIDE);
					dat->currentPage = i;
					if (opd)
						opd->pDialog->Show();
					return 0;
				}
			}

			if (LOWORD(wParam) == IDOK)
				DestroyWindow(hdlg);
		}
		break;

	case WM_DESTROY:
		if (FilterTimerId)
			KillTimer(hdlg, FilterTimerId);
		DestroyWindow(hFilterSearchWnd);
		ClearFilterStrings();
		dat->szFilterString[0] = 0;

		UnhookEvent(dat->hPluginLoad);
		UnhookEvent(dat->hPluginUnload);

		SaveOptionsTreeState(hdlg);
		Window_FreeIcon_IcoLib(hdlg);

		opd = dat->getCurrent();
		if (opd) {
			if (opd->ptszTab)
				db_set_ts(NULL, "Options", "LastTab", opd->ptszTab);
			else
				db_unset(NULL, "Options", "LastTab");
			if (opd->ptszGroup)
				db_set_ts(NULL, "Options", "LastGroup", opd->ptszGroup);
			else
				db_unset(NULL, "Options", "LastGroup");
			db_set_ts(NULL, "Options", "LastPage", opd->ptszTitle);
		}
		else {
			db_unset(NULL, "Options", "LastTab");
			db_unset(NULL, "Options", "LastGroup");
			db_unset(NULL, "Options", "LastPage");
		}

		Utils_SaveWindowPosition(hdlg, NULL, "Options", "");

		for (int i = 0; i < dat->arOpd.getCount(); i++)
			delete dat->arOpd[i];

		DeleteObject(dat->hBoldFont);
		delete dat;
		hwndOptions = NULL;

		CallService(MS_MODERNOPT_RESTORE, 0, 0);
		break;
	}
	return FALSE;
}

void OpenAccountOptions(PROTOACCOUNT *pa)
{
	if (pa->ppro == NULL)
		return;

	OptionsPageInit opi = { 0 };
	pa->ppro->OnEvent(EV_PROTO_ONOPTIONS, (WPARAM)&opi, 0);
	if (opi.pageCount == 0)
		return;

	TCHAR tszTitle[100];
	mir_sntprintf(tszTitle, SIZEOF(tszTitle), TranslateT("%s options"), pa->tszAccountName);

	OPENOPTIONSDIALOG ood = { sizeof(ood) };
	ood.pszGroup = LPGEN("Network");
	ood.pszPage = mir_t2a(pa->tszAccountName);

	PROPSHEETHEADER psh = { sizeof(psh) };
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	psh.hwndParent = NULL;
	psh.nPages = opi.pageCount;
	psh.pStartPage = (LPCTSTR)&ood;
	psh.pszCaption = tszTitle;
	psh.ppsp = (PROPSHEETPAGE*)opi.odp;
	hwndOptions = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OPTIONSPAGE), NULL, OptionsDlgProc, (LPARAM)&psh);
	mir_free((void*)ood.pszPage);
	FreeOptionsData(&opi);
}

static void OpenOptionsNow(int hLangpack, const char *pszGroup, const char *pszPage, const char *pszTab, bool bSinglePage = false)
{
	if (IsWindow(hwndOptions)) {
		ShowWindow(hwndOptions, SW_RESTORE);
		SetForegroundWindow(hwndOptions);
		if (pszPage != NULL) {
			ptrT ptszPage(mir_a2t(pszPage));
			HTREEITEM hItem = NULL;
			if (pszGroup != NULL) {
				ptrT ptszGroup(mir_a2t(pszGroup));
				hItem = FindNamedTreeItemAtRoot(GetDlgItem(hwndOptions, IDC_PAGETREE), TranslateTH(hLangpack, ptszGroup));
				if (hItem != NULL)
					hItem = FindNamedTreeItemAtChildren(GetDlgItem(hwndOptions, IDC_PAGETREE), hItem, TranslateTH(hLangpack, ptszPage));
			}
			else hItem = FindNamedTreeItemAtRoot(GetDlgItem(hwndOptions, IDC_PAGETREE), TranslateTH(hLangpack, ptszPage));

			if (hItem != NULL)
				TreeView_SelectItem(GetDlgItem(hwndOptions, IDC_PAGETREE), hItem);
		}
	}
	else {
		OptionsPageInit opi = { 0 };
		NotifyEventHooks(hOptionsInitEvent, (WPARAM)&opi, 0);
		if (opi.pageCount == 0)
			return;

		OPENOPTIONSDIALOG ood = { 0 };
		ood.cbSize = sizeof(ood);
		ood.pszGroup = pszGroup;
		ood.pszPage = pszPage;
		ood.pszTab = pszTab;

		PROPSHEETHEADER psh = { 0 };
		psh.dwSize = sizeof(psh);
		psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
		psh.nPages = opi.pageCount;
		psh.pStartPage = (LPCTSTR)&ood; // more structure misuse
		psh.pszCaption = TranslateT("Miranda NG options");
		psh.ppsp = (PROPSHEETPAGE*)opi.odp; // blatent misuse of the structure, but what the hell

		hwndOptions = CreateDialogParam(hInst,
			MAKEINTRESOURCE(bSinglePage ? IDD_OPTIONSPAGE : IDD_OPTIONS),
			NULL, OptionsDlgProc, (LPARAM)&psh);

		FreeOptionsData(&opi);
	}
}

static INT_PTR OpenOptions(WPARAM wParam, LPARAM lParam)
{
	OPENOPTIONSDIALOG *ood = (OPENOPTIONSDIALOG*)lParam;
	if (ood == NULL || ood->cbSize != sizeof(OPENOPTIONSDIALOG))
		return 1;
	
	OpenOptionsNow((int)wParam, ood->pszGroup, ood->pszPage, ood->pszTab);
	return 0;
}

static INT_PTR OpenOptionsPage(WPARAM wParam, LPARAM lParam)
{
	OPENOPTIONSDIALOG *ood = (OPENOPTIONSDIALOG*)lParam;
	if (ood == NULL || ood->cbSize != sizeof(OPENOPTIONSDIALOG))
		return 1;

	OpenOptionsNow((int)wParam, ood->pszGroup, ood->pszPage, ood->pszTab, true);
	return (INT_PTR)hwndOptions;
}

static INT_PTR OpenOptionsDialog(WPARAM, LPARAM)
{
	if (hwndOptions || !ServiceExists(MS_MODERNOPT_SHOW))
		OpenOptionsNow(NULL, NULL, NULL, NULL);
	else
		CallService(MS_MODERNOPT_SHOW, 0, 0);
	return 0;
}

static INT_PTR AddOptionsPage(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE *odp = (OPTIONSDIALOGPAGE*)lParam, *dst;
	OptionsPageInit *opi = (OptionsPageInit*)wParam;
	if (odp == NULL || opi == NULL)
		return 1;

	opi->odp = (OPTIONSDIALOGPAGE*)mir_realloc(opi->odp, sizeof(OPTIONSDIALOGPAGE)*(opi->pageCount + 1));
	dst = opi->odp + opi->pageCount;
	memcpy(dst, odp, sizeof(OPTIONSDIALOGPAGE));

	if (odp->ptszTitle != NULL) {
		if (odp->flags & ODPF_UNICODE)
			dst->ptszTitle = mir_wstrdup(odp->ptszTitle);
		else {
			dst->ptszTitle = mir_a2u(odp->pszTitle);
			dst->flags |= ODPF_UNICODE;
		}
	}

	if (odp->ptszGroup != NULL) {
		if (odp->flags & ODPF_UNICODE)
			dst->ptszGroup = mir_wstrdup(odp->ptszGroup);
		else {
			dst->ptszGroup = mir_a2t(odp->pszGroup);
			dst->flags |= ODPF_UNICODE;
		}
	}

	if (odp->ptszTab != NULL) {
		if (odp->flags & ODPF_UNICODE)
			dst->ptszTab = mir_wstrdup(odp->ptszTab);
		else {
			dst->ptszTab = mir_a2t(odp->pszTab);
			dst->flags |= ODPF_UNICODE;
		}
	}

	if ((DWORD_PTR)odp->pszTemplate & 0xFFFF0000)
		dst->pszTemplate = mir_strdup(odp->pszTemplate);

	opi->pageCount++;
	return 0;
}

static int OptModulesLoaded(WPARAM, LPARAM)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_OPTIONS);
	mi.position = 1900000000;
	mi.pszName = LPGEN("&Options...");
	mi.pszService = "Options/OptionsCommand";
	Menu_AddMainMenuItem(&mi);
	return 0;
}

int ShutdownOptionsModule(WPARAM, LPARAM)
{
	if (IsWindow(hwndOptions)) {
		DestroyWindow(hwndOptions);
		hwndOptions = NULL;
	}
	return 0;
}

int LoadOptionsModule(void)
{
	hwndOptions = NULL;
	hOptionsInitEvent = CreateHookableEvent(ME_OPT_INITIALISE);
	HookEvent(ME_OPT_INITIALISE, LangpackOptionsInit);

	CreateServiceFunction("Opt/AddPage", AddOptionsPage);
	CreateServiceFunction("Opt/OpenOptions", OpenOptions);
	CreateServiceFunction("Opt/OpenOptionsPage", OpenOptionsPage);
	CreateServiceFunction("Options/OptionsCommand", OpenOptionsDialog);
	HookEvent(ME_SYSTEM_MODULESLOADED, OptModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownOptionsModule);
	return 0;
}
