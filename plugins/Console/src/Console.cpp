/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#define MS_CONSOLE_SHOW_HIDE "Console/Show/Hide"

#define DEFAULT_WRAPLEN 90
#define MIN_WRAPLEN     25
#define MAX_WRAPLEN     255

#define MIN_LIMIT       1000
#define MAX_LIMIT       1000000

#define IMG_EMPTY       0
#define IMG_ARROW       1
#define IMG_IN          2
#define IMG_OUT         3
#define IMG_INFO        4

#define LOGICONX_SIZE  10
#define LOGICONY_SIZE  10

#define ICON_FIRST     3

#define ICON_NOSCROLL  1
#define ICON_PAUSED    2
#define ICON_SCROLL    3
#define ICON_STARTED   4

#define HM_DUMP        (WM_USER+10)
#define HM_ADD         (WM_USER+11)
#define HM_REMOVE      (WM_USER+12)
#define HM_SETFONT     (WM_USER+13)
#define HM_SETCOLOR    (WM_USER+14)
#define HM_PAUSEALL    (WM_USER+15)
#define HM_RESTART     (WM_USER+16)


typedef struct {
	const char *pszHead;
	const char *pszMsg;
} LOGMSG;


typedef struct {
	char szModule[128];
	TCHAR szMsg[1];
} DUMPMSG;


typedef struct {
	HWND hwnd;
	HWND hList;
	char *Module;
	int Scroll;
	int Paused;
	int newline;
} LOGWIN;



static SortedList lModules = { 0 };

static LOGWIN *pActive = NULL;
static int tabCount = 0;
static RECT rcTabs = { 0 };
static HWND hTabs = NULL;
static HWND hwndConsole = NULL;

static HIMAGELIST gImg = NULL;
static HFONT hfLogFont = NULL;
static COLORREF colLogFont;
static COLORREF colBackground;

static int gIcons = 0;
static int gVisible = 0;
static int gSingleMode = 0;
static int gLimit = 0;
static int gSeparator = 0;

static DWORD gWrapLen = DEFAULT_WRAPLEN;

static DWORD OutMsgs = 0;
static DWORD InMsgs = 0;

static HICON hIcons[15] = { 0 };
static HGENMENU hMenu = NULL;

static void LoadSettings();
static void ShowConsole(int show);
static INT_PTR ShowHideConsole(WPARAM wParam, LPARAM lParam);
static int Openfile(TCHAR *outputFile, int selection);

////////////////////////////////////////////////////////////////////////////////

static HANDLE hTTBButt = 0;

static int OnTTBLoaded(WPARAM, LPARAM)
{
	int state = IsWindowVisible(hwndConsole);

	TTBButton ttb = { sizeof(ttb) };
	ttb.hIconHandleUp = LoadIcon(IDI_BTN_UP);
	ttb.hIconHandleDn = LoadIcon(IDI_BTN_DN);
	ttb.dwFlags = (state ? TTBBF_PUSHED : 0) | TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_CONSOLE_SHOW_HIDE;
	ttb.name = LPGEN("Show/Hide Console");
	ttb.pszTooltipDn = LPGEN("Hide Console");
	ttb.pszTooltipUp = LPGEN("Show Console");
	hTTBButt = TopToolbar_AddButton(&ttb);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void ScrollDown(LOGWIN *dat) {
	if (dat->Scroll)
		ListView_EnsureVisible(dat->hList, ListView_GetItemCount(dat->hList) - 1, FALSE);
}

////////////////////////////////////////////////////////////////////////////////

static void ShowConsole(int show)
{
	HWND hwnd = NULL;

	if (!hwndConsole || !pActive) return;

	gVisible = show;

	if (show) {
		hwnd = GetForegroundWindow();
		if (InMsgs == OutMsgs)
			ScrollDown(pActive);
	}
	ShowWindow(hwndConsole, show ? SW_SHOW : SW_HIDE);
	db_set_b(NULL, "Console", "Show", (BYTE)(show ? 1 : 0));

	if (hwnd)
		SetForegroundWindow(hwnd);

	if (show)
		RedrawWindow(pActive->hList, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ERASE);

	if (hMenu) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.ptszName = show ? LPGENT("Hide Console") : LPGENT("Show Console");
		mi.flags = CMIM_NAME | CMIF_TCHAR;
		Menu_ModifyItem(hMenu, &mi);
	}

	if (hTTBButt)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTBButt, show ? TTBST_PUSHED : 0);
}

////////////////////////////////////////////////////////////////////////////////

static INT_PTR ShowHideConsole(WPARAM, LPARAM)
{
	if (hwndConsole)
		ShowConsole(!IsWindowVisible(hwndConsole));

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int LogResize(HWND hwnd, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_LIST:
		ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_LIST), 0, urc->dlgNewSize.cx - 25);
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	case IDC_STARTALL:
	case IDC_PAUSEALL:
	case IDC_CLOSE:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_TOP;
	default:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}
}

////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	UINT control;
	UINT icon;
	int  type;
	TCHAR *tooltip;
} controlinfo;


static controlinfo ctrls[] =
{
	// IDC_SCROLL & IDC_PAUSE must be first
	{IDC_SCROLL, IDI_SCROLL, BUTTONSETASFLATBTN, LPGENT("Scrolling (Ctrl+Q)")},
	{IDC_PAUSE, IDI_STARTED, BUTTONSETASFLATBTN, LPGENT("Pause logging (Ctrl+P)")},
	{IDC_SAVE, IDI_SAVE, BUTTONSETASFLATBTN, LPGENT("Save log to file (Ctrl+S)")},
	{IDC_COPY, IDI_COPY, BUTTONSETASFLATBTN, LPGENT("Copy selected log (Ctrl+C)")},
	{IDC_DELETE, IDI_DELETE, BUTTONSETASFLATBTN, LPGENT("Delete selected (Del)")},
	{IDC_OPTIONS, IDI_OPTIONS, BUTTONSETASFLATBTN, LPGENT("Log options (Ctrl+O)")},
	{IDC_STARTALL, IDI_START, BUTTONSETASFLATBTN, LPGENT("Start logging in all tabs")},
	{IDC_PAUSEALL, IDI_PAUSE, BUTTONSETASFLATBTN, LPGENT("Pause logging in all tabs")},
	{IDC_CLOSE, IDI_CLOSE, BUTTONSETASFLATBTN, LPGENT("Close tab (Ctrl+W)")},
};

////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int ctrl;

	switch (msg) {
	case WM_KEYDOWN:
		ctrl = GetKeyState(VK_CONTROL) & 0x8000;
		if (wParam == VK_DELETE && !ctrl) {
			SendMessage(GetParent(hwnd), WM_COMMAND, IDC_DELETE, 0);
			break;
		}

		if (wParam == VK_LEFT && ctrl) {
			NMHDR nmhdr = { 0 };
			int tab = TabCtrl_GetCurSel(hTabs);

			if (tab == 0)
				tab = TabCtrl_GetItemCount(hTabs) - 1;
			else
				tab--;

			TabCtrl_SetCurSel(hTabs, tab);
			nmhdr.code = TCN_SELCHANGE;
			SendMessage(hwndConsole, WM_NOTIFY, IDC_TABS, (LPARAM)&nmhdr);
			break;
		}

		if (wParam == VK_RIGHT && ctrl) {
			NMHDR nmhdr = { 0 };
			int tab = TabCtrl_GetCurSel(hTabs);
			int count =  TabCtrl_GetItemCount(hTabs);
			tab = (tab + 1) % count;

			TabCtrl_SetCurSel(hTabs, tab);
			nmhdr.code = TCN_SELCHANGE;
			SendMessage(hwndConsole, WM_NOTIFY, IDC_TABS, (LPARAM)&nmhdr);
			break;
		}

		break;

	case WM_CHAR:
		// CTRL
		if (!(GetKeyState(VK_CONTROL)&0x8000))
			break;

		switch (wParam) {

		case 1:	// Ctrl+A
			if (ListView_GetSelectedCount(hwnd) != (UINT)ListView_GetItemCount(hwnd))
				ListView_SetItemState(hwnd, -1, LVIS_SELECTED, LVIS_SELECTED);
			return 0;

		case 3: // Ctrl+D
			SendMessage(GetParent(hwnd), WM_COMMAND, IDC_COPY, 0);
			return 0;

		case 15: // Ctrl+O
			SendMessage(GetParent(hwnd), WM_COMMAND, IDC_OPTIONS, 0);
			return 0;

		case 16: // Ctrl+P
			SendMessage(GetParent(hwnd), WM_COMMAND, IDC_PAUSE, 0);
			return 0;

		case 17: // Ctrl+Q
			SendMessage(GetParent(hwnd), WM_COMMAND, IDC_SCROLL, 0);
			return 0;

		case 19: // Ctrl+S
			SendMessage(GetParent(hwnd), WM_COMMAND, IDC_SAVE, 0);
			return 0;

		case 23: // Ctrl+W
			SendMessage(GetParent(hwnd), WM_COMMAND, IDC_CLOSE, 0);
			return 0;
		}
		break;
	}
	return mir_callNextSubclass(hwnd, SubclassProc, msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK LogDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LOGWIN *dat = (LOGWIN *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:
		{
			dat = (LOGWIN *)lParam;

			dat->hwnd = hwndDlg;
			dat->Scroll = 1;
			dat->Paused = 0;
			dat->hList = GetDlgItem(hwndDlg, IDC_LIST);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			mir_subclassWindow(dat->hList, SubclassProc);

			// init buttons
			for (int i = 0; i < SIZEOF(ctrls); i++) {
				HWND hwnd = GetDlgItem(hwndDlg, ctrls[i].control);
				SendMessage(hwnd, ctrls[i].type, 0, 0);
				SendMessage(hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcons[i + ICON_FIRST]);
				SendMessage(hwnd, BUTTONADDTOOLTIP, (WPARAM)TranslateTS(ctrls[i].tooltip), BATF_TCHAR);
			}

			CheckDlgButton(hwndDlg, IDC_SCROLL, dat->Scroll ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_SCROLL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcons[(dat->Scroll ? ICON_SCROLL : ICON_NOSCROLL)]);

			if (gSingleMode) {
				ShowWindow(GetDlgItem(hwndDlg, IDC_PAUSEALL), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_STARTALL), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_CLOSE), SW_HIDE);
			}

			// init listview
			LVITEM lvi = { 0 };
			LVCOLUMN sLC;
			//ListView_SetUnicodeFormat(dat->hList, TRUE);
			ListView_SetImageList(dat->hList, gImg, LVSIL_SMALL);
			sLC.mask = LVCF_FMT | LVCF_WIDTH;
			sLC.fmt = LVCFMT_LEFT;
			sLC.cx = 630;
			ListView_InsertColumn(dat->hList, 0, &sLC);
			ListView_SetExtendedListViewStyle(dat->hList, LVS_EX_FULLROWSELECT);

			lvi.mask = LVIF_TEXT;
			if (gIcons) {
				lvi.mask |= LVIF_IMAGE;
				lvi.iImage = IMG_INFO;
			}

			lvi.pszText = TranslateT("*** Console started ***");
			ListView_InsertItem(dat->hList, &lvi);

			SendMessage(hwndDlg, WM_SIZE, 0, 0);
		}
		break;

	case HM_DUMP:
		if (!lParam) break;
		if (dat && !dat->Paused) {
			LVITEM lvi = { 0 };
			int last = 0x7fffffff;
			TCHAR szBreak;
			DWORD len, tmplen;
			DWORD wraplen = gWrapLen;
			TCHAR *str = ((DUMPMSG *)lParam)->szMsg;

			lvi.iItem = 0x7fffffff;

			str = _tcstok(str, _T("\n"));

			if (gIcons && str != NULL) {
				lvi.mask = LVIF_TEXT | LVIF_IMAGE;

				if (_tcsstr(str, _T("Data received"))) {
					if (gSeparator) ListView_InsertItem(dat->hList, &lvi);
					lvi.iImage = IMG_IN;
				}
				else if (_tcsstr(str, _T("Data sent"))) {
					if (gSeparator) ListView_InsertItem(dat->hList, &lvi);
					lvi.iImage = IMG_OUT;
				}
				else {
					if (gSeparator && dat->newline) {
						ListView_InsertItem(dat->hList, &lvi);
						dat->newline = 0;
					}
					lvi.iImage = IMG_INFO;
				}
			}
			else lvi.mask = LVIF_TEXT;

			while (str != NULL) {
				lvi.pszText = &str[0];
				tmplen = len = (DWORD)_tcslen(lvi.pszText);

				while (len > wraplen) {
					szBreak = lvi.pszText[wraplen];
					lvi.pszText[wraplen] = 0;
					last = ListView_InsertItem(dat->hList, &lvi);
					lvi.pszText[wraplen] = szBreak;
					len -= wraplen;
					lvi.pszText = &str[0] + tmplen - len;

					dat->newline = 1;
					lvi.iImage = IMG_EMPTY;
				}

				if (len && lvi.pszText[len-1] == '\r')
					lvi.pszText[len-1] = 0;

				last = ListView_InsertItem(dat->hList, &lvi);

				str = _tcstok(NULL, _T("\n"));

				if (str) dat->newline = 1;
				lvi.iImage = IMG_EMPTY;
			}

			if (gVisible && dat == pActive && dat->Scroll == 1)
				ListView_EnsureVisible(dat->hList, last, FALSE);

			if (last > gLimit) {
				int idx = last - gLimit + gLimit / 4; // leave only 75% of LIMIT

				while (idx >= 0) {
					ListView_DeleteItem(dat->hList, idx);
					idx--;
				}
			}
		}

		mir_free((DUMPMSG *)lParam);
		return TRUE;

	case WM_SIZE:
		{
			UTILRESIZEDIALOG urd = { sizeof(urd) };
			urd.hInstance = hInst;
			urd.hwndDlg = hwndDlg;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_LOG);
			urd.pfnResizer = LogResize;
			SetWindowPos(hwndDlg, HWND_TOP, rcTabs.left, rcTabs.top, rcTabs.right - rcTabs.left, rcTabs.bottom - rcTabs.top, SWP_SHOWWINDOW);
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
		}
		break;

	case WM_COMMAND:
		if (!dat)
			break;

		switch(LOWORD(wParam)) {
		case IDC_PAUSE:
			{
				LVITEM lvi = { 0 };
				dat->Paused = !(dat->Paused);
				lvi.mask = LVIF_TEXT | LVIF_IMAGE;
				lvi.iImage = IMG_INFO;
				lvi.iItem = 0x7FFFFFFF;
				lvi.pszText = (dat->Paused) ? TranslateT("*** Console paused ***") : TranslateT("*** Console resumed ***");
				ListView_InsertItem(dat->hList, &lvi);
				CheckDlgButton(hwndDlg, IDC_PAUSE, (dat->Paused) ? BST_CHECKED : BST_UNCHECKED);
				SendDlgItemMessage(hwndDlg, IDC_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcons[((dat->Paused) ? ICON_PAUSED : ICON_STARTED)]);
				break;
			}
		case IDC_SCROLL:
			dat->Scroll = !(dat->Scroll);
			CheckDlgButton(hwndDlg, IDC_SCROLL, (dat->Scroll) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_SCROLL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcons[((dat->Scroll) ? ICON_SCROLL : ICON_NOSCROLL)]);
			break;

		case IDC_COPY:
			{
				int idx = 0;
				TCHAR szText[128];
				TCHAR *src, *dst, *buf;
				int flags = LVNI_BELOW;
				int count = ListView_GetSelectedCount(dat->hList);

				if (count)
					flags |= LVNI_SELECTED;
				else
					count = ListView_GetItemCount(dat->hList);

				dst = buf = (TCHAR *)malloc((count * (sizeof(szText) + 1) + 1) * sizeof(TCHAR));
				if (!buf) break;

				while ((idx = ListView_GetNextItem(dat->hList, idx, flags)) > 0) {
					ListView_GetItemText(dat->hList, idx, 0, szText, SIZEOF(szText) - 1);
					src = szText;
					while (*dst++ = *src++);
					dst--;
					*dst++ = '\r';
					*dst++ = '\n';
					*dst = 0;
				}

				if (dst - buf > 0 && OpenClipboard(hwndDlg)) {
					EmptyClipboard();
					HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, (dst - buf + 1) * sizeof(TCHAR));
					if (hClipboardData) {
						TCHAR *pchData = (TCHAR *)GlobalLock(hClipboardData);
						_tcscpy(pchData, buf);
						GlobalUnlock(hClipboardData);
						SetClipboardData(CF_UNICODETEXT, hClipboardData);
					}
					CloseClipboard();
				}
				free(buf);
				break;
			}
		case IDC_DELETE:
			{
				int idx = 0;
				int count = ListView_GetSelectedCount(dat->hList);

				if (!count) break;

				if (count == ListView_GetItemCount(dat->hList)) {
					LVITEM lvi = { 0 };
					ListView_DeleteAllItems(dat->hList);
					lvi.mask = LVIF_TEXT | LVIF_IMAGE;
					lvi.iImage = IMG_INFO;
					lvi.pszText = TranslateT("*** Console cleared ***");
					ListView_InsertItem(dat->hList, &lvi);
					dat->newline = 0;
					break;
				}

				while ((idx = ListView_GetNextItem(dat->hList, idx, LVNI_BELOW | LVNI_SELECTED)) > 0) {
					ListView_DeleteItem(dat->hList, idx);
					idx--;
				}
			}
			break;

		case IDC_SAVE:
			{
				TCHAR szFile[MAX_PATH];

				if (!Openfile(szFile, ListView_GetSelectedCount(dat->hList))) break;

				FILE *fp = _tfopen(szFile, _T("wt"));
				if (fp) {
					int idx = 0;
					TCHAR szText[128];
					int flags = LVNI_BELOW;
					if (ListView_GetSelectedCount(dat->hList))
						flags |= LVNI_SELECTED;

					while ((idx = ListView_GetNextItem(dat->hList, idx, flags)) > 0) {
						ListView_GetItemText(dat->hList, idx, 0, szText, SIZEOF(szText));
						_ftprintf(fp, _T("%s\n"), szText);
					}
					fclose(fp);
				}
				break;
			}
		case IDC_OPTIONS:
			CallServiceSync(MS_NETLIB_LOGWIN, 0, 0);
			break;
		case IDC_STARTALL:
			SendMessage(hwndConsole, HM_PAUSEALL, 0, 0);
			break;
		case IDC_PAUSEALL:
			SendMessage(hwndConsole, HM_PAUSEALL, 0, 1);
			break;
		case IDC_CLOSE:
			if (tabCount > 1)
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		SendMessage(hwndConsole, HM_REMOVE, 0, (LPARAM)dat);
		break;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////////////

int ConsoleResize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_TABS:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	default:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}
}


static INT_PTR CALLBACK ConsoleDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
	{
		TCHAR title[MAX_PATH];
		TCHAR name[MAX_PATH] = { 0 };
		TCHAR path[MAX_PATH] = { 0 };

		hTabs = GetDlgItem(hwndDlg, IDC_TABS);

		//TabCtrl_SetMinTabWidth(hTabs, 100);

		// restore position
		Utils_RestoreWindowPositionEx(hwndDlg, RWPF_HIDDEN, NULL, "Console", "Console");

		CallService(MS_DB_GETPROFILENAMET, (WPARAM)SIZEOF(name), (LPARAM)name);

		CallService(MS_DB_GETPROFILEPATHT, (WPARAM)SIZEOF(path), (LPARAM)path);

		mir_sntprintf(title, SIZEOF(title), _T("%s - %s\\%s"), TranslateT("Miranda Console"), path, name);

		SetWindowText(hwndDlg, title);
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcons[0]);

		hwndConsole = hwndDlg;
		SendMessage(hwndDlg, HM_ADD, 0, 0);
		PostMessage(hwndDlg, WM_SIZE, 0, 0);
		break;
	}
	case HM_DUMP:
	{
		// lParam = DUMPMSG
		int idx;
		LOGWIN *lw;
		LOGWIN lw2;
		DUMPMSG *dumpMsg = (DUMPMSG *)lParam;

		if (!pActive) {
			mir_free(dumpMsg);
			break;
		}

		if (!gSingleMode) {
			lw2.Module = dumpMsg->szModule;
			if (!List_GetIndex(&lModules, &lw2, &idx))
				SendMessage(hwndDlg, HM_ADD, (WPARAM)idx, (LPARAM)dumpMsg->szModule);

			lw = (LOGWIN *)lModules.items[idx];
		}
		else
			lw = pActive;

		if (lw->hwnd)
			SendMessage(lw->hwnd, HM_DUMP, wParam, lParam);
		else
			PostMessage(hwndDlg, HM_DUMP, wParam, lParam); // loop msg until window will be ready

		return TRUE;
	}
	case HM_ADD:
	{
		// wParam = index, lParam = module name
		LOGWIN *lw;
		COLORREF col;
		TCITEM tci = { 0 };
		int idx = (int)wParam;
		char *str = (char *)lParam;

		if (!str) str = ""; // startup window

		lw = (LOGWIN *)mir_calloc(sizeof(LOGWIN));
		lw->Module = (char *)mir_strdup(str);
		List_Insert(&lModules, lw, idx);

		if (!gSingleMode && lParam) {
			tci.mask = TCIF_PARAM | TCIF_TEXT;
			tci.lParam = (LPARAM)lw;

			tci.pszText = mir_a2u(lw->Module);
			idx = TabCtrl_InsertItem(hTabs, tabCount, &tci);
			mir_free(tci.pszText);

			tabCount++;
		}

		GetClientRect(hTabs, &rcTabs);
		TabCtrl_AdjustRect(hTabs, FALSE, &rcTabs);

		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_LOG), hwndDlg, LogDlgProc, (LPARAM)lw);
		ShowWindow(lw->hwnd, (tabCount > 1) ? SW_HIDE : SW_SHOWNOACTIVATE);

		if (pActive)
		{
			col = ListView_GetBkColor(pActive->hList);
			ListView_SetBkColor(lw->hList, col);
			ListView_SetTextBkColor(lw->hList, col);

			col = ListView_GetTextColor(pActive->hList);
			ListView_SetTextColor(lw->hList, col);

			if (hfLogFont)
				SendMessage(lw->hList, WM_SETFONT, (WPARAM)hfLogFont, TRUE);
		}

		// hide startup window
		if (tabCount == 1) {
			ShowWindow(pActive->hwnd, SW_HIDE);
			PostMessage(pActive->hwnd, WM_CLOSE, 0, 0);
			pActive = lw;
		}

		if (!pActive)
			pActive = lw;

		return TRUE;
	}
	case HM_REMOVE:
	{
		// lParam = LOGWIN
		LOGWIN *lw = (LOGWIN *)lParam;

		if (!lw) break;

		if (lw == pActive) {
			int tab = TabCtrl_GetCurSel(hTabs);
			if (tab >= 0) {
				TCITEM tci = { 0 };

				TabCtrl_DeleteItem(hTabs, tab);
				tabCount--;
				if (tabCount) {
					tab--;
					if (tab < 0 ) tab = 0;
					TabCtrl_SetCurSel(hTabs, tab);

					tci.mask = TCIF_PARAM;
					TabCtrl_GetItem(hTabs, tab, &tci);
					pActive = (LOGWIN *)tci.lParam;
					SendMessage(pActive->hwnd, WM_SIZE, 0, 0);
					ScrollDown(pActive);
					ShowWindow(pActive->hwnd, SW_SHOWNOACTIVATE);
					SetFocus(pActive->hList);
				}
				else
					pActive = NULL;
			}
		}

		List_RemovePtr(&lModules, lw);
		mir_free(lw->Module);
		mir_free(lw);
		return TRUE;
	}
	case HM_SETFONT:
	{
		// wParam = font, lParam = font color
		int i;
		LOGWIN *lw;
		for (i = 0; i < lModules.realCount; i++) {
			lw = (LOGWIN *)lModules.items[i];
			ListView_SetTextColor(lw->hList, (COLORREF)lParam);
			if (wParam)
				SendMessage(lw->hList, WM_SETFONT, wParam, TRUE);
		}
		return TRUE;
	}
	case HM_SETCOLOR:
	{
		// wParam = font, lParam = background color
		int i;
		LOGWIN *lw;
		for (i = 0; i < lModules.realCount; i++) {
			lw = (LOGWIN *)lModules.items[i];
			ListView_SetBkColor(lw->hList, (COLORREF)lParam);
			ListView_SetTextBkColor(lw->hList, (COLORREF)lParam);
			if (wParam)
				SendMessage(lw->hList, WM_SETFONT, wParam, TRUE);
		}
		return TRUE;
	}
	case HM_PAUSEALL:
	{
		// lParam = 1 to pause, 0 to start
		int i;
		LOGWIN *lw;
		for (i = 0; i < lModules.realCount; i++) {
			lw = (LOGWIN *)lModules.items[i];
			if (lw->Paused != (int)lParam)
				SendMessage(lw->hwnd, WM_COMMAND, IDC_PAUSE, 0);
		}
		return TRUE;
	}
	case HM_RESTART:
	{
		if (pActive) {
			pActive = NULL;
			PostMessage(hwndDlg, HM_RESTART, 0, 0);
			return TRUE;
		}
		// close all tabs
		if (!lParam) {
			LOGWIN *lw;
			TabCtrl_DeleteAllItems(hTabs);
			while (lModules.realCount) {
				lw = (LOGWIN *)lModules.items[0];
				SendMessage(lw->hwnd, WM_CLOSE, 0, 0);
			}
			tabCount = 0;
			PostMessage(hwndDlg, HM_RESTART, 0, 1);
			return TRUE;
		}

		LoadSettings();
		SendMessage(hwndDlg, HM_ADD, 0, 0);
		PostMessage(hwndDlg, WM_SIZE, 0, 0);
		SendMessage(hwndDlg, HM_SETFONT, (WPARAM)hfLogFont, (LPARAM)colLogFont);
		SendMessage(hwndDlg, HM_SETCOLOR, (WPARAM)hfLogFont, (LPARAM)colBackground);
		return TRUE;
	}
 	case WM_SETFOCUS:
		if (pActive) {
			SetFocus(pActive->hList);
		}
		return TRUE;
	case WM_NOTIFY:
		switch (wParam) {
		case IDC_TABS:
			{
				LPNMHDR lpnmhdr = (LPNMHDR)lParam;
				if (lpnmhdr->code == TCN_SELCHANGE) {
					int newTab = TabCtrl_GetCurSel(hTabs);
					if (newTab >= 0) {
						TCITEM tci = { 0 };
						HWND hOld = pActive->hwnd;

						tci.mask = TCIF_PARAM;

						if (!TabCtrl_GetItem(hTabs, newTab, &tci)) break;

						pActive = (LOGWIN *)tci.lParam;

						SendMessage(pActive->hwnd, WM_SIZE, 0, 0);
						ScrollDown(pActive);
						ShowWindow(hOld, SW_HIDE);
						ShowWindow(pActive->hwnd, SW_SHOWNOACTIVATE);
						SetFocus(pActive->hList);
					}
					else
						SendMessage(pActive->hwnd, WM_SIZE, 0, 0);
				}
				break;
			}
		}
		break;
	case WM_SIZE:
	{
		UTILRESIZEDIALOG urd = { 0 };
		urd.cbSize = sizeof(urd);
		urd.hInstance = hInst;
		urd.hwndDlg = hwndDlg;
		urd.lpTemplate = MAKEINTRESOURCEA(IDD_CONSOLE);
		urd.pfnResizer = ConsoleResize;
		CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);

		GetClientRect(hTabs, &rcTabs);
		TabCtrl_AdjustRect(hTabs, FALSE, &rcTabs);

		if (pActive)
			SendMessage(pActive->hwnd, WM_SIZE, 0, 0);
		break;
	}
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO *mmi = (MINMAXINFO *)lParam;
		mmi->ptMinTrackSize.x = 400;
		mmi->ptMinTrackSize.y = 200;
		break;
	}
	case WM_CLOSE:
		if (lParam != 1) {
			Utils_SaveWindowPosition(hwndDlg, NULL, "Console", "Console");
			ShowConsole(0);
			return TRUE;
		}
		else
			DestroyWindow(hwndDlg);
		break;
	case WM_DESTROY:
		pActive = NULL;
		if (hfLogFont) DeleteObject(hfLogFont);
		PostQuitMessage(0);
		break;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////////////


void __cdecl ConsoleThread(void*)
{
	MSG msg;
	HWND hwnd;

	hwnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CONSOLE), NULL, ConsoleDlgProc);

	if (!hwnd) return;

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		switch (msg.message) {
		case HM_DUMP:
			OutMsgs++;
			break;
		}

		if (hwnd != NULL && IsDialogMessage(hwnd, &msg)) /* Wine fix. */
			continue;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	hwndConsole = NULL;
}

///////////////////////////////////////////////////////////////////////////////

static int OnFastDump(WPARAM wParam, LPARAM lParam)
{
	if (pActive) {
		LOGMSG *logMsg = (LOGMSG *)lParam;
		DWORD headlen = (DWORD)strlen(logMsg->pszHead);
		DWORD msglen = (DWORD)strlen(logMsg->pszMsg);
		DWORD len = (headlen + msglen + 1) * sizeof(TCHAR) + sizeof(DUMPMSG);
		DUMPMSG *dumpMsg = (DUMPMSG *)mir_alloc(len);
		TCHAR *str = dumpMsg->szMsg;

		char *szModule = (wParam) ? ((NETLIBUSER *)wParam)->szDescriptiveName : "[Core]";
		strncpy_s(dumpMsg->szModule, SIZEOF(dumpMsg->szModule), szModule, _TRUNCATE);

		wchar_t *ucs2 = mir_a2u(logMsg->pszHead);
		wcscpy(str, ucs2);
		mir_free(ucs2);

		// try to detect utf8
		ucs2 = mir_utf8decodeW(logMsg->pszMsg);
		if (!ucs2)
			ucs2 = mir_a2u(logMsg->pszMsg);

		wcscat(str, ucs2);
		mir_free(ucs2);

		InMsgs++;
		PostMessage(hwndConsole, HM_DUMP, 0, (LPARAM)dumpMsg);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static void LoadSettings()
{
	gIcons = db_get_b(NULL, "Console", "ShowIcons", 1);
	gSeparator = db_get_b(NULL, "Console", "Separator", 1);
	gSingleMode = db_get_b(NULL, "Console", "SingleMode", 0);

	gWrapLen = db_get_b(NULL, "Console", "Wrap", DEFAULT_WRAPLEN);
	if (gWrapLen < MIN_WRAPLEN) gWrapLen = DEFAULT_WRAPLEN;

	gLimit = db_get_dw(NULL, "Console", "Limit", MIN_LIMIT);
	if (gLimit > MAX_LIMIT) gLimit = MAX_LIMIT;
	if (gLimit < MIN_LIMIT) gLimit = MIN_LIMIT;
}


static void SaveSettings(HWND hwndDlg)
{
	int len = GetDlgItemInt(hwndDlg, IDC_WRAP, NULL, FALSE);
	if (len < MIN_WRAPLEN )
		len = MIN_WRAPLEN;
	else if (len > MAX_WRAPLEN)
		len = MAX_WRAPLEN;

	gWrapLen = len;
	SetDlgItemInt(hwndDlg, IDC_WRAP, gWrapLen, FALSE);
	db_set_b(NULL, "Console", "Wrap", (BYTE)len);

	len = GetDlgItemInt(hwndDlg, IDC_LIMIT, NULL, FALSE);
	if (len < MIN_LIMIT )
		len = MIN_LIMIT;
	else if (len > MAX_LIMIT)
		len = MAX_LIMIT;

	gLimit = len;
	SetDlgItemInt(hwndDlg, IDC_LIMIT, gLimit, FALSE);
	db_set_dw(NULL, "Console", "Limit", len);

	db_set_b(NULL, "Console", "SingleMode", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SINGLE));
	db_set_b(NULL, "Console", "Separator", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SEPARATOR));
	db_set_b(NULL, "Console", "ShowIcons", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWICONS));

	db_set_b(NULL, "Console", "ShowAtStart", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_START));
}


static INT_PTR CALLBACK OptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_START, db_get_b(NULL, "Console", "ShowAtStart", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SINGLE, gSingleMode ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWICONS, gIcons ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SEPARATOR, gSeparator ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg, IDC_WRAP, gWrapLen, FALSE);
		SetDlgItemInt(hwndDlg, IDC_LIMIT, gLimit, FALSE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RESTART:
			{
				if (!pActive) break;
				SaveSettings(hwndDlg);
				PostMessage(hwndConsole, HM_RESTART, 0, 0);
				break;
			}
		case IDC_LIMIT:
		case IDC_WRAP:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return FALSE;
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_START:
		case IDC_SEPARATOR:
		case IDC_SHOWICONS:
		case IDC_SINGLE:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				{
					SaveSettings(hwndDlg);
					break;
				}
			}
			break;
		}
		break;
	}
	return FALSE;
}


static int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszGroup = LPGEN("Services");
	odp.pszTitle = LPGEN("Console");
	odp.pfnDlgProc = OptDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int OnColourChange(WPARAM, LPARAM)
{
	if (hwndConsole) {
		ColourID cid = { 0 };

		cid.cbSize = sizeof(cid);
		strcpy(cid.group, "Console");
		strcpy(cid.name, "Background");
		strcpy(cid.dbSettingsGroup, "Console");
		strcpy(cid.setting, "BgColor");

		colBackground = (COLORREF)CallService(MS_COLOUR_GET, (WPARAM)&cid, 0);
		if (colBackground != -1)
			SendMessage(hwndConsole, HM_SETCOLOR, (WPARAM)hfLogFont, (LPARAM)colBackground);
	}
	return 0;
}


static int OnFontChange(WPARAM, LPARAM)
{
	if (hwndConsole) {
		HFONT hf = NULL;
		LOGFONT LogFont = { 0 };
		FontIDT fid = { 0 };
		fid.cbSize = sizeof(fid);

		_tcsncpy(fid.group, LPGENT("Console"), SIZEOF(fid.group));
		_tcsncpy(fid.name, LPGENT("Text"), SIZEOF(fid.name));

		colLogFont = (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&LogFont);

		if (LogFont.lfHeight != 0) {
			hf = CreateFontIndirect(&LogFont);

			SendMessage(hwndConsole, HM_SETFONT, (WPARAM)hf, (LPARAM)colLogFont);

			if (hfLogFont)
				DeleteObject(hfLogFont);
			hfLogFont = hf;
		}
	}
	return 0;
}

static int OnSystemModulesLoaded(WPARAM, LPARAM)
{
	CreateServiceFunction(MS_CONSOLE_SHOW_HIDE, ShowHideConsole);

	FontIDT fid = { 0 };
	fid.cbSize = sizeof(fid);
	_tcsncpy(fid.group, LPGENT("Console"), SIZEOF(fid.group));
	_tcsncpy(fid.name, LPGENT("Text"), SIZEOF(fid.name));
	strncpy(fid.dbSettingsGroup, "Console", SIZEOF(fid.dbSettingsGroup));
	strncpy(fid.prefix, "ConsoleFont", SIZEOF(fid.prefix));
	_tcsncpy(fid.backgroundGroup, LPGENT("Console"), SIZEOF(fid.backgroundGroup));
	_tcsncpy(fid.backgroundName, LPGENT("Background"), SIZEOF(fid.backgroundName));
	fid.flags = FIDF_DEFAULTVALID;
	fid.deffontsettings.charset = DEFAULT_CHARSET;
	fid.deffontsettings.colour = RGB(0, 0, 0);
	fid.deffontsettings.size = 10;
	fid.deffontsettings.style = 0;
	_tcsncpy(fid.deffontsettings.szFace, _T("Courier"), SIZEOF(fid.deffontsettings.szFace));
	FontRegisterT(&fid);

	HookEvent(ME_FONT_RELOAD, OnFontChange);

	ColourIDT cid = { 0 };
	cid.cbSize = sizeof(cid);
	_tcsncpy(cid.group, LPGENT("Console"), SIZEOF(cid.group));
	_tcsncpy(cid.name, LPGENT("Background"), SIZEOF(cid.name));
	strncpy(cid.dbSettingsGroup, "Console", SIZEOF(cid.dbSettingsGroup));
	strncpy(cid.setting, "BgColor", SIZEOF(cid.setting));
	cid.defcolour = RGB(255, 255, 255);
	ColourRegisterT(&cid);

	HookEvent(ME_COLOUR_RELOAD, OnColourChange);

	HOTKEYDESC hkd = { 0 };
	hkd.cbSize = sizeof(hkd);
	hkd.pszName = "Console_Show_Hide";
	hkd.pszDescription = LPGEN("Show/Hide Console");
	hkd.pszSection = "Main";
	hkd.pszService = MS_CONSOLE_SHOW_HIDE;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_EXT, 'C');
	Hotkey_Register(&hkd);

	if (hwndConsole && IsWindow(hwndConsole)) {
		HookEvent(ME_TTB_MODULELOADED, OnTTBLoaded);

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIF_TCHAR;
		mi.hIcon = hIcons[0];
		mi.position = 1900000000;
		mi.ptszName = (IsWindowVisible(hwndConsole)) ? LPGENT("Hide Console") : LPGENT("Show Console");
		mi.pszService = MS_CONSOLE_SHOW_HIDE;
		hMenu = Menu_AddMainMenuItem(&mi);

		OnFontChange(0, 0);
		OnColourChange(0, 0);

		if (db_get_b(NULL, "Console", "ShowAtStart", 0) || db_get_b(NULL, "Console", "Show", 1))
			ShowConsole(1);
		else
			ShowConsole(0);
	}

	return 0;
}

static int PreshutdownConsole(WPARAM, LPARAM)
{
	if (hwndConsole)
		PostMessage(hwndConsole, WM_CLOSE, 0, 1 );

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int stringCompare( LOGWIN *lw1, LOGWIN *lw2 )
{
	return strcmp( lw1->Module, lw2->Module );
}

///////////////////////////////////////////////////////////////////////////////

static UINT logicons[] = {IDI_EMPTY, IDI_ARROW, IDI_IN, IDI_OUT, IDI_INFO};


void InitConsole()
{
	int i;
	HICON hi;

	lModules.sortFunc = (FSortFunc)stringCompare;
	lModules.increment = 5;

	hIcons[0] = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CONSOLE));
	hIcons[1] = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_NOSCROLL),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
	hIcons[2] = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_PAUSED),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);

	for(i = 0; i < SIZEOF(ctrls); i++) {
		hIcons[i+ICON_FIRST] = (HICON)LoadImage(hInst,MAKEINTRESOURCE(ctrls[i].icon),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
	}

	gImg = ImageList_Create(LOGICONX_SIZE, LOGICONY_SIZE, ILC_COLOR24 | ILC_MASK, SIZEOF(logicons), 0);

	for(i = 0; i < SIZEOF(logicons); i++)
	{
		hi = (HICON)LoadImage(hInst,MAKEINTRESOURCE(logicons[i]),IMAGE_ICON,LOGICONX_SIZE,LOGICONY_SIZE,0);
		if (hi)
		{
			ImageList_AddIcon(gImg, hi);
			DestroyIcon(hi);
		}
	}

	LoadSettings();

	mir_forkthread(ConsoleThread, 0);

	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreshutdownConsole);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_NETLIB_FASTDUMP, OnFastDump);
}

void ShutdownConsole(void)
{
	int i;

	List_Destroy(&lModules);

	if (gImg) ImageList_Destroy(gImg);

	for(i = 0; i < SIZEOF(hIcons); i++) {
		if (hIcons[i]) DestroyIcon(hIcons[i]);
	}
}

////////////////////////////////////////////////////////////////////////////////

TCHAR *addstring(TCHAR *str, TCHAR *add) {
	_tcscpy(str,add);
	return str + _tcslen(add) + 1;
}


static int Openfile(TCHAR *outputFile, int selection)
{
	TCHAR filename[MAX_PATH+2] = _T("");
	TCHAR *title;

	TCHAR *filter, *tmp, *tmp1, *tmp2;
	tmp1 = TranslateT("Text Files (*.txt)");
	tmp2 = TranslateT("All Files");
	filter = tmp = (TCHAR*)_alloca((_tcslen(tmp1)+_tcslen(tmp2)+11)*sizeof(TCHAR));
	tmp = addstring(tmp, tmp1);
	tmp = addstring(tmp, _T("*.TXT"));
	tmp = addstring(tmp, tmp2);
	tmp = addstring(tmp, _T("*"));
	*tmp = 0;

	if (selection)
		title = TranslateT("Save selection to file");
	else
		title = TranslateT("Save log to file");

	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwndConsole;
	ofn.lpstrFile = filename;
	ofn.lpstrFilter = filter;
	ofn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrTitle = title;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = _T("txt");

	if (!GetSaveFileName(&ofn))
		return 0;
	_tcscpy(outputFile, filename);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
