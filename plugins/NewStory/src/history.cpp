////////////////////////////////////////////////////////////////////////
// NewStory -- new history viewer for Miranda IM
// (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
// Visit http://miranda-im.org/ for details on Miranda Instant Messenger
////////////////////////////////////////////////////////////////////////

/*
for the date picker:
    case WM_ACTIVATE:
        if (wParam == WA_INACTIVE) PostMessage(m_hwndDialog, WM_CLOSE, 0, 0);
        break;
*/

#include "stdafx.h"

MWindowList hNewstoryWindows = 0;

int evtEventAdded(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = WindowList_Find(hNewstoryWindows, (UINT_PTR)wParam);
	SendMessage(hwnd, UM_ADDEVENT, wParam, lParam);
	return 0;
}

int evtEventDeleted(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = WindowList_Find(hNewstoryWindows, (UINT_PTR)wParam);
	SendMessage(hwnd, UM_REMOVEEVENT, wParam, lParam);
	return 0;
}

void InitHistory()
{
	hNewstoryWindows = WindowList_Create();

	HookEvent(ME_DB_EVENT_ADDED, evtEventAdded);
	HookEvent(ME_DB_EVENT_DELETED, evtEventDeleted);
}

void FreeHistory()
{
}

enum
{
	HIST_SHOW_IN = 0x001,
	HIST_SHOW_OUT = 0x002,
	HIST_SHOW_MSGS = 0x004,
	HIST_SHOW_FILES = 0x008,
	HIST_SHOW_URLS = 0x010,
	HIST_SHOW_STATUS = 0x020,
	HIST_SHOW_OTHER = 0x040,
	HIST_AUTO_FILTER = 0x080,
};

enum
{
	WND_OPT_TIMETREE = 0x01,
	WND_OPT_SEARCHBAR = 0x02,
	WND_OPT_FILTERBAR = 0x04
};

enum
{
	WND_SPACING = 4,
	TBTN_SIZE = 25,
	TBTN_SPACER = 10
};

enum
{
	TBTN_USERINFO, TBTN_USERMENU, TBTN_MESSAGE,
	TBTN_SEARCH, TBTN_FILTER, TBTN_DATEPOPUP,
	TBTN_COPY, TBTN_EXPORT,
	TBTN_LOGOPTIONS, TBTN_SECURITY, TBTN_CLOSE,
	TBTN_COUNT
};

int tbtnSpacing[TBTN_COUNT] = { 0, 0, TBTN_SPACER, 0, 0, TBTN_SPACER, 0, -1, 0, 0, 0 };

struct InfoBarEvents
{
	HWND hwndIco, hwndIcoIn, hwndIcoOut;
	HWND hwndTxt, hwndTxtIn, hwndTxtOut;
};

struct WindowData
{
	HMENU hMenu;
	WORD showFlags;
	bool gonnaRedraw;
	bool isContactHistory;
	MCONTACT hContact;
	int lastYear, lastMonth, lastDay;
	HTREEITEM hLastYear, hLastMonth, hLastDay;
	bool disableTimeTreeChange;

	// window flags
	DWORD wndOptions;

	// toolbar buttons
	HWND hwndBtnToolbar[TBTN_COUNT];
	// main controls
	HWND hwndTimeTree;
	HWND hwndLog;
	// searchbar
	HWND hwndBtnCloseSearch, hwndBtnFindNext, hwndBtnFindPrev;
	HWND hwndSearchText;
	// statusbar
	HWND hwndStatus;
	// filter bar
	HWND hwndChkDateFrom, hwndChkDateTo;
	HWND hwndDateFrom, hwndDateTo;
	InfoBarEvents ibMessages, ibFiles, ibUrls, ibTotal;
};

void LayoutFilterBar(HDWP hDwp, int x, int y, int w, InfoBarEvents *ib)
{
	hDwp = DeferWindowPos(hDwp, ib->hwndIco, 0,
		x, y, 16, 16, SWP_NOZORDER);
	hDwp = DeferWindowPos(hDwp, ib->hwndTxt, 0,
		x + 16 + WND_SPACING, y, w - 16 - WND_SPACING, 16, SWP_NOZORDER);

	hDwp = DeferWindowPos(hDwp, ib->hwndIcoIn, 0,
		x + 16, y + 16 + WND_SPACING, 16, 16, SWP_NOZORDER);
	hDwp = DeferWindowPos(hDwp, ib->hwndTxtIn, 0,
		x + 32 + WND_SPACING, y + 16 + WND_SPACING, w - WND_SPACING - 32, 16, SWP_NOZORDER);

	hDwp = DeferWindowPos(hDwp, ib->hwndIcoOut, 0,
		x + 16, y + (16 + WND_SPACING) * 2, 16, 16, SWP_NOZORDER);
	hDwp = DeferWindowPos(hDwp, ib->hwndTxtOut, 0,
		x + 32 + WND_SPACING, y + (16 + WND_SPACING) * 2, w - WND_SPACING - 32, 16, SWP_NOZORDER);

}

void ShowHideControls(HWND hwnd, WindowData *data)
{
	int cmd;

	cmd = (data->wndOptions & WND_OPT_FILTERBAR) ? SW_SHOW : SW_HIDE;
	ShowWindow(data->ibMessages.hwndIco, cmd);
	ShowWindow(data->ibMessages.hwndIcoIn, cmd);
	ShowWindow(data->ibMessages.hwndIcoOut, cmd);
	ShowWindow(data->ibMessages.hwndTxt, cmd);
	ShowWindow(data->ibMessages.hwndTxtIn, cmd);
	ShowWindow(data->ibMessages.hwndTxtOut, cmd);
	ShowWindow(data->ibFiles.hwndIco, cmd);
	ShowWindow(data->ibFiles.hwndIcoIn, cmd);
	ShowWindow(data->ibFiles.hwndIcoOut, cmd);
	ShowWindow(data->ibFiles.hwndTxt, cmd);
	ShowWindow(data->ibFiles.hwndTxtIn, cmd);
	ShowWindow(data->ibFiles.hwndTxtOut, cmd);
	ShowWindow(data->ibUrls.hwndIco, cmd);
	ShowWindow(data->ibUrls.hwndIcoIn, cmd);
	ShowWindow(data->ibUrls.hwndIcoOut, cmd);
	ShowWindow(data->ibUrls.hwndTxt, cmd);
	ShowWindow(data->ibUrls.hwndTxtIn, cmd);
	ShowWindow(data->ibUrls.hwndTxtOut, cmd);
	ShowWindow(data->ibTotal.hwndIco, cmd);
	ShowWindow(data->ibTotal.hwndIcoIn, cmd);
	ShowWindow(data->ibTotal.hwndIcoOut, cmd);
	ShowWindow(data->ibTotal.hwndTxt, cmd);
	ShowWindow(data->ibTotal.hwndTxtIn, cmd);
	ShowWindow(data->ibTotal.hwndTxtOut, cmd);
	ShowWindow(data->hwndDateFrom, cmd);
	ShowWindow(data->hwndDateTo, cmd);
	ShowWindow(data->hwndChkDateFrom, cmd);
	ShowWindow(data->hwndChkDateTo, cmd);
	ShowWindow(GetDlgItem(hwnd, IDC_IB_SEPARATOR), cmd);

	cmd = (data->wndOptions & WND_OPT_SEARCHBAR) ? SW_SHOW : SW_HIDE;
	ShowWindow(data->hwndBtnCloseSearch, cmd);
	ShowWindow(data->hwndBtnFindNext, cmd);
	ShowWindow(data->hwndBtnFindPrev, cmd);
	ShowWindow(data->hwndSearchText, cmd);
}

void LayoutHistoryWnd(HWND hwnd, WindowData *data)
{
	int i;
	RECT rc;
	GetClientRect(hwnd, &rc);
	int x, y; // tmp vars
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	HDWP hDwp = BeginDeferWindowPos(50);

	// toolbar
	int hToolBar = TBTN_SIZE + WND_SPACING;
	x = WND_SPACING;
	int btnReverse = -1;
	for (i = 0; i < TBTN_COUNT; ++i) {
		hDwp = DeferWindowPos(hDwp, data->hwndBtnToolbar[i], 0,
			x, WND_SPACING,
			TBTN_SIZE, TBTN_SIZE,
			SWP_NOZORDER);
		x += TBTN_SIZE + tbtnSpacing[i];
		if (tbtnSpacing[i] < 0) {
			btnReverse = i;
			break;
		}
	}
	x = w - WND_SPACING - TBTN_SIZE;
	for (i = TBTN_COUNT - 1; i > btnReverse; --i) {
		hDwp = DeferWindowPos(hDwp, data->hwndBtnToolbar[i], 0,
			x, WND_SPACING,
			TBTN_SIZE, TBTN_SIZE,
			SWP_NOZORDER);
		x -= TBTN_SIZE + tbtnSpacing[i - 1];
	}

	// infobar
//	hDwp = DeferWindowPos(hDwp, data->hwndIcoProtocol, 0,
//		w-100+WND_SPACING, WND_SPACING,
//		16, 16,
//		SWP_NOZORDER);
//	hDwp = DeferWindowPos(hDwp, data->hwndTxtNickname, 0,
//		w-100+WND_SPACING*2+16, WND_SPACING,
//		100, 16,
//		SWP_NOZORDER);
//	hDwp = DeferWindowPos(hDwp, data->hwndTxtUID, 0,
//		w-100+WND_SPACING*2+16, WND_SPACING*2+16,
//		100, 16,
//		SWP_NOZORDER);

	// filter bar
	int hFilterBar = 0;
	if (data->wndOptions & WND_OPT_FILTERBAR) {
		hFilterBar = WND_SPACING + (16 + WND_SPACING) * 3;
		LayoutFilterBar(hDwp, WND_SPACING + (WND_SPACING + 75) * 0, WND_SPACING * 2 + hToolBar, 75, &data->ibMessages);
		LayoutFilterBar(hDwp, WND_SPACING + (WND_SPACING + 75) * 1, WND_SPACING * 2 + hToolBar, 75, &data->ibFiles);
		LayoutFilterBar(hDwp, WND_SPACING + (WND_SPACING + 75) * 2, WND_SPACING * 2 + hToolBar, 75, &data->ibUrls);
		LayoutFilterBar(hDwp, WND_SPACING + (WND_SPACING + 75) * 3, WND_SPACING * 2 + hToolBar, 75, &data->ibTotal);

		GetWindowRect(data->hwndChkDateFrom, &rc);
		x = rc.right - rc.left;
		GetWindowRect(data->hwndDateFrom, &rc);
		y = hToolBar + WND_SPACING + (WND_SPACING + (16 + WND_SPACING) * 3 - (rc.bottom - rc.top) * 2 - WND_SPACING) / 2;
		hDwp = DeferWindowPos(hDwp, data->hwndChkDateFrom, 0,
			w - x - (rc.right - rc.left) - WND_SPACING * 2, y,
			x, rc.bottom - rc.top,
			SWP_NOZORDER);
		hDwp = DeferWindowPos(hDwp, data->hwndDateFrom, 0,
			w - (rc.right - rc.left) - WND_SPACING, y,
			rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOZORDER);

		hDwp = DeferWindowPos(hDwp, data->hwndChkDateTo, 0,
			w - x - (rc.right - rc.left) - WND_SPACING * 2, y + (rc.bottom - rc.top) + WND_SPACING,
			x, rc.bottom - rc.top,
			SWP_NOZORDER);
		hDwp = DeferWindowPos(hDwp, data->hwndDateTo, 0,
			w - (rc.right - rc.left) - WND_SPACING, y + (rc.bottom - rc.top) + WND_SPACING,
			rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOZORDER);

		hDwp = DeferWindowPos(hDwp, GetDlgItem(hwnd, IDC_IB_SEPARATOR), 0,
			WND_SPACING, hToolBar + WND_SPACING,
			w - WND_SPACING * 2, 2,
			SWP_NOZORDER);
	}

	// general
	GetWindowRect(data->hwndStatus, &rc);
	int hStatus = rc.bottom - rc.top;
	hDwp = DeferWindowPos(hDwp, data->hwndStatus, 0,
		0, h - hStatus,
		w, hStatus,
		SWP_NOZORDER);

	int hSearch = 0;
	if (data->wndOptions & WND_OPT_SEARCHBAR) {
		GetWindowRect(data->hwndSearchText, &rc);
		hSearch = rc.bottom - rc.top;
		hDwp = DeferWindowPos(hDwp, data->hwndBtnCloseSearch, 0,
			WND_SPACING, h - hSearch - hStatus - WND_SPACING,
			TBTN_SIZE, hSearch, SWP_NOZORDER);
		hDwp = DeferWindowPos(hDwp, data->hwndSearchText, 0,
			TBTN_SIZE + WND_SPACING * 2, h - hSearch - hStatus - WND_SPACING,
			w - WND_SPACING * 4 - TBTN_SIZE * 3, hSearch,
			SWP_NOZORDER);
		hDwp = DeferWindowPos(hDwp, data->hwndBtnFindPrev, 0,
			w - WND_SPACING - TBTN_SIZE * 2, h - hSearch - hStatus - WND_SPACING,
			TBTN_SIZE, hSearch,
			SWP_NOZORDER);
		hDwp = DeferWindowPos(hDwp, data->hwndBtnFindNext, 0,
			w - WND_SPACING - TBTN_SIZE * 1, h - hSearch - hStatus - WND_SPACING,
			TBTN_SIZE, hSearch,
			SWP_NOZORDER);
		hSearch += WND_SPACING;
	}

	hDwp = DeferWindowPos(hDwp, data->hwndLog, 0,
		WND_SPACING, hToolBar + hFilterBar + WND_SPACING,
		w - WND_SPACING * 2, h - WND_SPACING * 2 - hFilterBar - hToolBar - hSearch - hStatus,
		SWP_NOZORDER);

	EndDeferWindowPos(hDwp);
	//	InvalidateRect(hwnd, 0, FALSE);
}

/*
bool ExportHistoryDialog(HANDLE hContact, HWND hwndHistory)
{
	int filterIndex = 0;
	char *filter = 0;
	char filterSize = 0;

	char *templates[100] = {0};
	int nTemplates = 0;

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile("plugins\\newstory\\x_*.txt", &ffd);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		char *fn = (char *)malloc(MAX_PATH);
		wsprintf(fn, "plugins\\newstory\\%s", ffd.cFileName);

		char *szSignature = "newstory export template";
		char line[1024];
		FILE *f = fopen(fn, "r");
		fgets(line, 1024, f);
		if (*line) line[lstrlen(line)-1] = 0;
		if (!lstrcmp(line, szSignature))
		{
			fgets(line, 1024, f);
			if (*line) line[lstrlen(line)-1] = 0;

			char *title = strdup(Translate(line));

			fgets(line, 1024, f);
			if (*line) line[lstrlen(line)-1] = 0;
			char *ext = line;

			// <title> (*.<ext>)\0*.<ext>\0
			int newFilterSize = filterSize + lstrlen(title) + 2*lstrlen(ext) + 9;
			char *newFilter = (char *)calloc(newFilterSize+1, 1);

			if (filterSize)
				memcpy(newFilter, filter, filterSize);

			char buf[1024];
			wsprintf(buf, "%s (*.%s)%c*.%s%c", title, ext, '\0', ext, '\0');
			memcpy(newFilter+filterSize, buf, newFilterSize-filterSize);

			free(filter);
			filter = newFilter;
			filterSize = newFilterSize;

			templates[nTemplates++] = fn;

			free(title);
		} else
		{
			free(fn);
		}
		fclose(f);

		if (!FindNextFile(hFind, &ffd))
			break;
	}


	char filename[MAX_PATH] = {0};

	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwndHistory;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = filter;
	ofn.lpstrCustomFilter = 0;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = filterIndex;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = 0;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = 0;
	ofn.lpstrTitle = Translate("Export History...");
	ofn.Flags = OFN_ENABLESIZING|OFN_LONGNAMES|OFN_NOCHANGEDIR|OFN_NOREADONLYRETURN|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = 0;
	ofn.lCustData = 0;
	ofn.lpfnHook = 0;
	ofn.lpTemplateName = 0;
	ofn.FlagsEx = 0;

	if (GetSaveFileName(&ofn))
	{
//		ofn.nFilterIndex;
		ExportHistory(hContact, templates[ofn.nFilterIndex-1], ofn.lpstrFile, hwndHistory);
	}

	for (int i = 0; i < 100; i++)
		if (templates[i])
			free(templates[i]);

	return false;
}
*/

INT_PTR CALLBACK HistoryDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	//CallSnappingWindowProc(hwnd, msg, wParam, lParam);

	WindowData *data = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if ((msg >= NSM_FIRST) && (msg < NSM_LAST)) {
		int result = SendMessage(GetDlgItem(hwnd, IDC_ITEMS2), msg, wParam, lParam);
		SetWindowLongPtr(hwnd, DWLP_MSGRESULT, result);
		return result;
	}

	switch (msg) {
	case WM_INITDIALOG:
		{
			data = new WindowData;
			data->hContact = (MCONTACT)lParam;
			data->disableTimeTreeChange = false;
			data->showFlags = db_get_w(data->hContact, MODULENAME, "showFlags", 0x7f);
			data->lastYear = data->lastMonth = data->lastDay = -1;
			data->hLastYear = data->hLastMonth = data->hLastDay = 0;

			data->wndOptions = 0;

			// get handles
			data->hwndBtnToolbar[TBTN_USERINFO] = GetDlgItem(hwnd, IDC_USERINFO);
			data->hwndBtnToolbar[TBTN_USERMENU] = GetDlgItem(hwnd, IDC_USERMENU);
			data->hwndBtnToolbar[TBTN_MESSAGE] = GetDlgItem(hwnd, IDC_MESSAGE);
			data->hwndBtnToolbar[TBTN_SEARCH] = GetDlgItem(hwnd, IDC_SEARCH);
			data->hwndBtnToolbar[TBTN_COPY] = GetDlgItem(hwnd, IDC_COPY);
			data->hwndBtnToolbar[TBTN_EXPORT] = GetDlgItem(hwnd, IDC_EXPORT);
			data->hwndBtnToolbar[TBTN_LOGOPTIONS] = GetDlgItem(hwnd, IDC_LOGOPTIONS);
			data->hwndBtnToolbar[TBTN_FILTER] = GetDlgItem(hwnd, IDC_FILTER);
			data->hwndBtnToolbar[TBTN_DATEPOPUP] = GetDlgItem(hwnd, IDC_DATEPOPUP);
			data->hwndBtnToolbar[TBTN_SECURITY] = GetDlgItem(hwnd, IDC_SECURITY);
			data->hwndBtnToolbar[TBTN_CLOSE] = GetDlgItem(hwnd, IDC_CLOSE);
			data->hwndLog = GetDlgItem(hwnd, IDC_ITEMS2);
			data->hwndBtnCloseSearch = GetDlgItem(hwnd, IDC_SEARCHICON);
			data->hwndBtnFindPrev = GetDlgItem(hwnd, IDC_FINDPREV);
			data->hwndBtnFindNext = GetDlgItem(hwnd, IDC_FINDNEXT);
			data->hwndSearchText = GetDlgItem(hwnd, IDC_SEARCHTEXT);
			data->hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hwnd, NULL, g_plugin.getInst(), NULL);
			SendMessage(data->hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);

			// filterbar
			SendMessage(data->hwndBtnToolbar[TBTN_FILTER], BUTTONSETASPUSHBTN, 0, 0);
			SendMessage(data->hwndBtnToolbar[TBTN_SEARCH], BUTTONSETASPUSHBTN, 0, 0);

			data->hwndChkDateFrom = GetDlgItem(hwnd, IDC_CHK_DATE_FROM);
			data->hwndChkDateTo = GetDlgItem(hwnd, IDC_CHK_DATE_TO);
			data->hwndDateFrom = GetDlgItem(hwnd, IDC_DATE_FROM);
			data->hwndDateTo = GetDlgItem(hwnd, IDC_DATE_TO);

			data->ibMessages.hwndIco = GetDlgItem(hwnd, IDC_ICO_MESSAGES);
			data->ibMessages.hwndTxt = GetDlgItem(hwnd, IDC_TXT_MESSAGES);
			data->ibMessages.hwndIcoIn = GetDlgItem(hwnd, IDC_ICO_MESSAGES_IN);
			SendMessage(data->ibMessages.hwndIcoIn, BUTTONSETASFLATBTN, 0, 0);
			SendMessage(data->ibMessages.hwndIcoIn, BUTTONSETASPUSHBTN, 0, 0);
			SendMessage(data->ibMessages.hwndIcoIn, BM_SETCHECK, BST_CHECKED, 0);
			data->ibMessages.hwndTxtIn = GetDlgItem(hwnd, IDC_TXT_MESSAGES_IN);
			data->ibMessages.hwndIcoOut = GetDlgItem(hwnd, IDC_ICO_MESSAGES_OUT);
			SendMessage(data->ibMessages.hwndIcoOut, BUTTONSETASFLATBTN, 0, 0);
			SendMessage(data->ibMessages.hwndIcoOut, BUTTONSETASPUSHBTN, 0, 0);
			SendMessage(data->ibMessages.hwndIcoOut, BM_SETCHECK, BST_CHECKED, 0);
			data->ibMessages.hwndTxtOut = GetDlgItem(hwnd, IDC_TXT_MESSAGES_OUT);

			data->ibFiles.hwndIco = GetDlgItem(hwnd, IDC_ICO_FILES);
			data->ibFiles.hwndTxt = GetDlgItem(hwnd, IDC_TXT_FILES);
			data->ibFiles.hwndIcoIn = GetDlgItem(hwnd, IDC_ICO_FILES_IN);
			SendMessage(data->ibFiles.hwndIcoIn, BUTTONSETASFLATBTN, 0, 0);
			SendMessage(data->ibFiles.hwndIcoIn, BUTTONSETASPUSHBTN, 0, 0);
			SendMessage(data->ibFiles.hwndIcoIn, BM_SETCHECK, BST_CHECKED, 0);
			data->ibFiles.hwndTxtIn = GetDlgItem(hwnd, IDC_TXT_FILES_IN);
			data->ibFiles.hwndIcoOut = GetDlgItem(hwnd, IDC_ICO_FILES_OUT);
			SendMessage(data->ibFiles.hwndIcoOut, BUTTONSETASFLATBTN, 0, 0);
			SendMessage(data->ibFiles.hwndIcoOut, BUTTONSETASPUSHBTN, 0, 0);
			SendMessage(data->ibFiles.hwndIcoOut, BM_SETCHECK, BST_CHECKED, 0);
			data->ibFiles.hwndTxtOut = GetDlgItem(hwnd, IDC_TXT_FILES_OUT);

			data->ibUrls.hwndIco = GetDlgItem(hwnd, IDC_ICO_URLS);
			data->ibUrls.hwndTxt = GetDlgItem(hwnd, IDC_TXT_URLS);
			data->ibUrls.hwndIcoIn = GetDlgItem(hwnd, IDC_ICO_URLS_IN);
			SendMessage(data->ibUrls.hwndIcoIn, BUTTONSETASFLATBTN, 0, 0);
			SendMessage(data->ibUrls.hwndIcoIn, BUTTONSETASPUSHBTN, 0, 0);
			SendMessage(data->ibUrls.hwndIcoIn, BM_SETCHECK, BST_CHECKED, 0);
			data->ibUrls.hwndTxtIn = GetDlgItem(hwnd, IDC_TXT_URLS_IN);
			data->ibUrls.hwndIcoOut = GetDlgItem(hwnd, IDC_ICO_URLS_OUT);
			SendMessage(data->ibUrls.hwndIcoOut, BUTTONSETASFLATBTN, 0, 0);
			SendMessage(data->ibUrls.hwndIcoOut, BUTTONSETASPUSHBTN, 0, 0);
			SendMessage(data->ibUrls.hwndIcoOut, BM_SETCHECK, BST_CHECKED, 0);
			data->ibUrls.hwndTxtOut = GetDlgItem(hwnd, IDC_TXT_URLS_OUT);

			data->ibTotal.hwndIco = GetDlgItem(hwnd, IDC_ICO_TOTAL);
			data->ibTotal.hwndTxt = GetDlgItem(hwnd, IDC_TXT_TOTAL);
			data->ibTotal.hwndIcoIn = GetDlgItem(hwnd, IDC_ICO_TOTAL_IN);
			SendMessage(data->ibTotal.hwndIcoIn, BUTTONSETASFLATBTN, 0, 0);
			SendMessage(data->ibTotal.hwndIcoIn, BUTTONSETASPUSHBTN, 0, 0);
			SendMessage(data->ibTotal.hwndIcoIn, BM_SETCHECK, BST_CHECKED, 0);
			data->ibTotal.hwndTxtIn = GetDlgItem(hwnd, IDC_TXT_TOTAL_IN);
			data->ibTotal.hwndIcoOut = GetDlgItem(hwnd, IDC_ICO_TOTAL_OUT);
			SendMessage(data->ibTotal.hwndIcoOut, BUTTONSETASFLATBTN, 0, 0);
			SendMessage(data->ibTotal.hwndIcoOut, BUTTONSETASPUSHBTN, 0, 0);
			SendMessage(data->ibTotal.hwndIcoOut, BM_SETCHECK, BST_CHECKED, 0);
			data->ibTotal.hwndTxtOut = GetDlgItem(hwnd, IDC_TXT_TOTAL_OUT);

			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);

			data->hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_POPUPS));
			//CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)data->hMenu, 0);
			CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_INCOMING,
				data->showFlags&HIST_SHOW_IN ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_OUTGOING,
				data->showFlags&HIST_SHOW_OUT ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_MESSAGES,
				data->showFlags&HIST_SHOW_MSGS ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_FILES,
				data->showFlags&HIST_SHOW_FILES ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_URLS,
				data->showFlags&HIST_SHOW_URLS ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_STATUS,
				data->showFlags&HIST_SHOW_STATUS ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_OTHER,
				data->showFlags&HIST_SHOW_OTHER ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_AUTO,
				data->showFlags&HIST_AUTO_FILTER ? MF_CHECKED : MF_UNCHECKED);

			//			CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_LOGOPTIONS_SHOWTIMETREE,
			//				data->showFlags&HIST_TIMETREE ? MF_CHECKED : MF_UNCHECKED);
			//			ShowWindow(GetDlgItem(hwnd, IDC_TIMETREE), data->showFlags&HIST_TIMETREE ? SW_SHOW : SW_HIDE);

						// Ask for layout
			PostMessage(hwnd, WM_SIZE, 0, 0);

			SendMessage(GetDlgItem(hwnd, IDC_USERINFO), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_MESSAGE), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_USERMENU), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_COPY), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_LOGOPTIONS), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_FILTER), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_DATEPOPUP), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_SEARCH), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_EXPORT), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_CLOSE), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_SECURITY), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_FINDPREV), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwnd, IDC_FINDNEXT), BUTTONSETASFLATBTN, 0, 0);

			SendMessage(GetDlgItem(hwnd, IDC_USERINFO), BUTTONADDTOOLTIP, (WPARAM)Translate("User Info"), 0);
			SendMessage(GetDlgItem(hwnd, IDC_MESSAGE), BUTTONADDTOOLTIP, (WPARAM)Translate("Send Message"), 0);
			SendMessage(GetDlgItem(hwnd, IDC_USERMENU), BUTTONADDTOOLTIP, (WPARAM)Translate("User Menu"), 0);
			SendMessage(GetDlgItem(hwnd, IDC_COPY), BUTTONADDTOOLTIP, (WPARAM)Translate("Copy"), 0);
			SendMessage(GetDlgItem(hwnd, IDC_LOGOPTIONS), BUTTONADDTOOLTIP, (WPARAM)Translate("Options"), 0);
			SendMessage(GetDlgItem(hwnd, IDC_FILTER), BUTTONADDTOOLTIP, (WPARAM)Translate("Filter"), 0);
			SendMessage(GetDlgItem(hwnd, IDC_DATEPOPUP), BUTTONADDTOOLTIP, (WPARAM)Translate("Jump2Date"), 0);
			SendMessage(GetDlgItem(hwnd, IDC_SEARCH), BUTTONADDTOOLTIP, (WPARAM)Translate("Search..."), 0);
			SendMessage(GetDlgItem(hwnd, IDC_EXPORT), BUTTONADDTOOLTIP, (WPARAM)Translate("Export..."), 0);
			SendMessage(GetDlgItem(hwnd, IDC_CLOSE), BUTTONADDTOOLTIP, (WPARAM)Translate("Close"), 0);
			SendMessage(GetDlgItem(hwnd, IDC_SECURITY), BUTTONADDTOOLTIP, (WPARAM)Translate("Security Options"), 0);
			SendMessage(GetDlgItem(hwnd, IDC_FINDPREV), BUTTONADDTOOLTIP, (WPARAM)Translate("Find Previous"), 0);
			SendMessage(GetDlgItem(hwnd, IDC_FINDNEXT), BUTTONADDTOOLTIP, (WPARAM)Translate("Find Next"), 0);

			WindowList_Add(hNewstoryWindows, hwnd, data->hContact);

			if (data->hContact && (data->hContact != INVALID_CONTACT_ID)) {
				TCHAR *title = TplFormatString(TPL_TITLE, data->hContact, 0);
				SetWindowText(hwnd, title);
				free(title);
			}
			else
				if (data->hContact == INVALID_CONTACT_ID) {
					SetWindowText(hwnd, TranslateT("Newstory Search Results"));
				}
				else {
					SetWindowText(hwnd, TranslateT("System Newstory"));
				}

			if (data->hContact != INVALID_CONTACT_ID) {
				//				ShowWindow(GetDlgItem(hwnd, IDC_TIMETREE), SW_HIDE);
				//				ShowWindow(GetDlgItem(hwnd, IDC_ITEMS), SW_HIDE);
				//				ShowWindow(GetDlgItem(hwnd, IDC_ITEMS2), SW_HIDE);
				//				ShowWindow(GetDlgItem(hwnd, IDC_SEARCHICON), SW_HIDE);
				PostMessage(GetDlgItem(hwnd, IDC_ITEMS2), WM_USER, (WPARAM)data->hContact, 0);
			}

			SendMessage(hwnd, UM_UPDATEICONS, 0, 0);
			SetFocus(GetDlgItem(hwnd, IDC_ITEMS2));

			int left = db_get_dw(data->hContact, MODULENAME, "left"),
				top = db_get_dw(data->hContact, MODULENAME, "top"),
				right = db_get_dw(data->hContact, MODULENAME, "right"),
				bottom = db_get_dw(data->hContact, MODULENAME, "bottom");

			if (left - right && top - bottom)
				MoveWindow(hwnd, left, top, right - left, bottom - top, TRUE);

			ShowHideControls(hwnd, data);

			return TRUE;
		}

		/*case WM_MOUSEWHEEL:
			SendMessage(GetDlgItem(hwnd, IDC_CUSTOM1), msg, wParam, lParam);
			return TRUE;*/

			/*case WM_SETFOCUS:
				SetFocus(GetDlgItem(hwnd, IDC_CUSTOM1));
				return TRUE;*/

	case UM_UPDATEICONS:
		SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)GetIcon(ICO_NEWSTORY));

		SendMessage(GetDlgItem(hwnd, IDC_SEARCHICON), STM_SETICON, (WPARAM)GetIcon(ICO_SEARCH), 0);

		SendMessage(GetDlgItem(hwnd, IDC_USERINFO), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_USERINFO));
		SendMessage(GetDlgItem(hwnd, IDC_MESSAGE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_SENDMSG));
		SendMessage(GetDlgItem(hwnd, IDC_USERMENU), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_USERMENU));
		SendMessage(GetDlgItem(hwnd, IDC_COPY), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_COPY));
		SendMessage(GetDlgItem(hwnd, IDC_LOGOPTIONS), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_OPTIONS));
		SendMessage(GetDlgItem(hwnd, IDC_FILTER), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_FILTER));
		SendMessage(GetDlgItem(hwnd, IDC_DATEPOPUP), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_CALENDAR));
		SendMessage(GetDlgItem(hwnd, IDC_SEARCH), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_SEARCH));
		SendMessage(GetDlgItem(hwnd, IDC_EXPORT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_EXPORT));
		SendMessage(GetDlgItem(hwnd, IDC_CLOSE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_CLOSE));
		SendMessage(GetDlgItem(hwnd, IDC_FINDPREV), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_FINDPREV));
		SendMessage(GetDlgItem(hwnd, IDC_FINDNEXT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_FINDNEXT));

		SendMessage(data->ibMessages.hwndIco, STM_SETICON, (LPARAM)GetIcon(ICO_SENDMSG), 0);
		SendMessage(data->ibMessages.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_MSGIN));
		SendMessage(data->ibMessages.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_MSGOUT));
		SendMessage(data->ibFiles.hwndIco, STM_SETICON, (LPARAM)GetIcon(ICO_FILE), 0);
		SendMessage(data->ibFiles.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_MSGIN));
		SendMessage(data->ibFiles.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_MSGOUT));
		SendMessage(data->ibUrls.hwndIco, STM_SETICON, (LPARAM)GetIcon(ICO_URL), 0);
		SendMessage(data->ibUrls.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_MSGIN));
		SendMessage(data->ibUrls.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_MSGOUT));
		SendMessage(data->ibTotal.hwndIco, STM_SETICON, (LPARAM)GetIcon(ICO_UNKNOWN), 0);
		SendMessage(data->ibTotal.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_MSGIN));
		SendMessage(data->ibTotal.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_MSGOUT));

		if (CheckPassword(data->hContact, ""))
			SendMessage(GetDlgItem(hwnd, IDC_SECURITY), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_NOPASSWORD));
		else
			SendMessage(GetDlgItem(hwnd, IDC_SECURITY), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_PASSWORD));

		break;

	case UM_REBUILDLIST:
		//			if (data->showFlags & HIST_TIMETREE)
		//				ShowWindow(GetDlgItem(hwnd, IDC_TIMETREE), SW_SHOW);
		//			ShowWindow(GetDlgItem(hwnd, IDC_ITEMS2), SW_SHOW);
		//			ShowWindow(GetDlgItem(hwnd, IDC_SEARCHICON), SW_SHOW);

		return TRUE;

		/*
				case UM_JUMP2TIME:
				{
					for (int i = 0; i < data->eventCount; i++)
					{
						ItemData *idata = (ItemData *)SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_GETITEMDATA, i, 0);
						if (idata->dbe->timestamp >= wParam)
						{
							SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SETCARETINDEX, i, 0);
							SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SETTOPINDEX, i, 0);
							SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SELITEMRANGE, FALSE, MAKELPARAM(0,data->eventCount));
							SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SELITEMRANGE, TRUE, MAKELPARAM(i,i));
							break;
						}
					}
					return TRUE;
				}
		*/

	case WM_MEASUREITEM:
		LPMEASUREITEMSTRUCT lpmis;
		lpmis = (LPMEASUREITEMSTRUCT)lParam;

		if (lpmis->CtlType == ODT_MENU)
			return Menu_MeasureItem(lParam);

		lpmis->itemHeight = 25;
		return TRUE;

	case WM_SIZE:
		LayoutHistoryWnd(hwnd, data);
		return TRUE;

	case WM_CHARTOITEM:
		if (!((GetKeyState(VK_CONTROL) & 0x80) || (GetKeyState(VK_MENU) & 0x80))) {
			TCHAR s[] = { LOWORD(wParam), 0 };
			SetWindowText(GetDlgItem(hwnd, IDC_SEARCHTEXT), s);
			SendMessage(GetDlgItem(hwnd, IDC_SEARCHTEXT), EM_SETSEL, 1, 1);
			SetFocus(GetDlgItem(hwnd, IDC_SEARCHTEXT));
		}
		return -1;

	case WM_CLOSE:
		WindowList_Remove(hNewstoryWindows, hwnd);

		db_set_dw(data->hContact, MODULENAME, "showFlags", data->showFlags);
		GetWindowRect(hwnd, &rc);
		db_set_dw(data->hContact, MODULENAME, "left", rc.left);
		db_set_dw(data->hContact, MODULENAME, "top", rc.top);
		db_set_dw(data->hContact, MODULENAME, "right", rc.right);
		db_set_dw(data->hContact, MODULENAME, "bottom", rc.bottom);

		//			CLCombo_Cleanup(GetDlgItem(hwnd, IDC_USERLIST));

		DestroyMenu(data->hMenu);
		delete data;
		DestroyWindow(hwnd);
		return TRUE;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis;
			lpdis = (LPDRAWITEMSTRUCT)lParam;

			if (lpdis->CtlType == ODT_MENU)
				return Menu_DrawItem(lParam);

			if (lpdis->itemID == -1)
				return FALSE;

			return TRUE;
		}

	case WM_NOTIFY:
		{
			LPNMHDR hdr = (LPNMHDR)lParam;
			switch (hdr->idFrom) {
			case IDC_TIMETREE:
				{
					switch (hdr->code) {
					case TVN_SELCHANGED:
						{
							if (data->disableTimeTreeChange) {
								data->disableTimeTreeChange = false;
							}
							else {
								//								LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
								//								int id = pnmtv->itemNew.lParam;
								//								SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SETCARETINDEX, id, 0);
								//								SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_ITEMS, LBN_SELCHANGE), (LPARAM)GetDlgItem(hwnd, IDC_ITEMS));
								//								SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SETTOPINDEX, id, 0);
								//								SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SELITEMRANGE, FALSE, MAKELPARAM(0,data->eventCount));
								//								SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SELITEMRANGE, TRUE, MAKELPARAM(id,id));
							}
							break;
						}
					}
					break;
				}
			}
		}
		return TRUE;

	case WM_COMMAND:
		// if (Menu_ProcessCommand(MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM) data->hContact))
		//    return TRUE;
			
		switch (LOWORD(wParam)) {
		case IDCANCEL:
		case IDC_CLOSE:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;

		case IDC_MESSAGE:
			CallService(MS_MSG_SENDMESSAGE, (WPARAM)data->hContact, 0);
			break;

		case IDC_USERINFO:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)data->hContact, 0);
			break;

		case IDC_DATEPOPUP:
			{
				GetWindowRect(GetDlgItem(hwnd, LOWORD(wParam)), &rc);
				time_t tm_jump = CalendarTool_Show(hwnd, rc.left, rc.bottom);
				if (tm_jump) PostMessage(hwnd, UM_JUMP2TIME, tm_jump, 0);
				break;
			}

		case IDC_USERMENU:
			{
				HMENU hMenu = Menu_BuildContactMenu(data->hContact);
				GetWindowRect(GetDlgItem(hwnd, LOWORD(wParam)), &rc);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwnd, NULL);
				DestroyMenu(hMenu);
				break;
			}

		case IDC_LOGOPTIONS:
			{
				GetWindowRect(GetDlgItem(hwnd, LOWORD(wParam)), &rc);
				//					DWORD itemID = 0;
				switch (TrackPopupMenu(GetSubMenu(data->hMenu, 2), TPM_RETURNCMD, rc.left, rc.bottom, 0, hwnd, NULL)) {
					//						case ID_LOGOPTIONS_SHOWTIMETREE:
					//						{
					//							data->showFlags = toggleBit(data->showFlags, HIST_TIMETREE);
					//							CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_LOGOPTIONS_SHOWTIMETREE,
					//								data->showFlags&HIST_TIMETREE ? MF_CHECKED : MF_UNCHECKED);
					//							ShowWindow(GetDlgItem(hwnd, IDC_TIMETREE), data->showFlags&HIST_TIMETREE ? SW_SHOW : SW_HIDE);
					//							break;
					//						}

				case ID_LOGOPTIONS_OPTIONS:
					g_plugin.openOptions(nullptr, L"Newstory", L"General");
					break;

				case ID_LOGOPTIONS_TEMPLATES:
					g_plugin.openOptions(nullptr, L"Newstory", L"Templates");
					break;

				case ID_LOGOPTIONS_PASSWORDS:
					g_plugin.openOptions(nullptr, L"Newstory", L"Passwords (not ready yet)");
					break;
				}
				PostMessage(hwnd, WM_SIZE, 0, 0);
				break;
			}

		case IDC_SEARCH:
			if (data->wndOptions & WND_OPT_SEARCHBAR)
				data->wndOptions &= ~WND_OPT_SEARCHBAR;
			else
				data->wndOptions |= WND_OPT_SEARCHBAR;

			ShowHideControls(hwnd, data);
			LayoutHistoryWnd(hwnd, data);
			break;

		case IDC_FILTER:
			if (data->wndOptions & WND_OPT_FILTERBAR)
				data->wndOptions &= ~WND_OPT_FILTERBAR;
			else
				data->wndOptions |= WND_OPT_FILTERBAR;

			ShowHideControls(hwnd, data);
			LayoutHistoryWnd(hwnd, data);
			break;

			/*
			GetWindowRect(GetDlgItem(hwnd, LOWORD(wParam)), &rc);
//					DWORD itemID = 0;
			bool doFilter = true;
			switch (TrackPopupMenu(GetSubMenu(data->hMenu, 1), TPM_RETURNCMD, rc.left, rc.bottom, 0, hwnd, NULL))
			{
				case ID_FILTER_INCOMING:
				{
					data->showFlags = toggleBit(data->showFlags, HIST_SHOW_IN);
					CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_INCOMING,
						data->showFlags&HIST_SHOW_IN ? MF_CHECKED : MF_UNCHECKED);
					break;
				}
				case ID_FILTER_OUTGOING:
				{
					data->showFlags = toggleBit(data->showFlags, HIST_SHOW_OUT);
					CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_OUTGOING,
						data->showFlags&HIST_SHOW_OUT ? MF_CHECKED : MF_UNCHECKED);
					break;
				}
				case ID_FILTER_MESSAGES:
				{
					data->showFlags = toggleBit(data->showFlags, HIST_SHOW_MSGS);
					CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_MESSAGES,
						data->showFlags&HIST_SHOW_MSGS ? MF_CHECKED : MF_UNCHECKED);
					break;
				}
				case ID_FILTER_FILES:
				{
					data->showFlags = toggleBit(data->showFlags, HIST_SHOW_FILES);
					CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_FILES,
						data->showFlags&HIST_SHOW_FILES ? MF_CHECKED : MF_UNCHECKED);
					break;
				}
				case ID_FILTER_URLS:
				{
					data->showFlags = toggleBit(data->showFlags, HIST_SHOW_URLS);
					CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_URLS,
						data->showFlags&HIST_SHOW_URLS ? MF_CHECKED : MF_UNCHECKED);
					break;
				}
				case ID_FILTER_STATUS:
				{
					data->showFlags = toggleBit(data->showFlags, HIST_SHOW_STATUS);
					CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_STATUS,
						data->showFlags&HIST_SHOW_STATUS ? MF_CHECKED : MF_UNCHECKED);
					break;
				}
				case ID_FILTER_OTHER:
				{
					data->showFlags = toggleBit(data->showFlags, HIST_SHOW_OTHER);
					CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_OTHER,
						data->showFlags&HIST_SHOW_OTHER ? MF_CHECKED : MF_UNCHECKED);
					break;
				}
				case ID_FILTER_AUTO:
				{
					data->showFlags = toggleBit(data->showFlags, HIST_AUTO_FILTER);
					CheckMenuItem(GetSubMenu(data->hMenu, 1), ID_FILTER_AUTO,
						data->showFlags&HIST_AUTO_FILTER ? MF_CHECKED : MF_UNCHECKED);
					break;
				}
				default:
				{
					doFilter = false;
					break;
				}
			}
			if (doFilter)
				PostMessage(hwnd, UM_REBUILDLIST, 0, 0);
			break;*/
	
		case IDC_SECURITY:
			ChangePassword(hwnd, data->hContact);
			PostMessage(hwnd, UM_UPDATEICONS, 0, 0);
			break;

		case IDC_EXPORT:
			// ExportHistoryDialog(data->hContact, hwnd);
			// DialogBox(hInst, MAKEINTRESOURCE(IDD_EXPORT), hwnd, ExportWndProc);
			break;

		case IDC_SEARCHTEXT:
			if ((data->showFlags&HIST_AUTO_FILTER) && (HIWORD(wParam) == EN_CHANGE))
				PostMessage(hwnd, UM_REBUILDLIST, 0, 0);
			break;

		// case IDC_EXPORT:
		//	GetWindowRect(GetDlgItem(hwnd, LOWORD(wParam)), &rc);
		//	TrackPopupMenu(GetSubMenu(data->hMenu, 0), TPM_RETURNCMD, rc.left, rc.bottom, 0, hwnd, NULL);
		//	break;
		
		// case IDC_SEARCH:
		// int id = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SEARCH), 0, SearchDlgProc, (LPARAM)GetDlgItem(hwnd, IDC_ITEMS));
		// SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SETCARETINDEX, id, 0);
		// SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SETTOPINDEX, id, 0);
		// SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SELITEMRANGE, FALSE, MAKELPARAM(0,data->eventCount));
		// SendMessage(GetDlgItem(hwnd, IDC_ITEMS), LB_SELITEMRANGE, TRUE, MAKELPARAM(id,id));
		// break;

		case IDC_FINDPREV:
			{
				int bufSize = GetWindowTextLength(GetDlgItem(hwnd, IDC_SEARCHTEXT)) + 1;
				TCHAR *buf = new TCHAR[bufSize];
				GetWindowText(GetDlgItem(hwnd, IDC_SEARCHTEXT), buf, GetWindowTextLength(GetDlgItem(hwnd, IDC_SEARCHTEXT)) + 1);
				SendMessage(GetDlgItem(hwnd, IDC_ITEMS2), NSM_FINDPREV, (WPARAM)buf, 0);
				delete[] buf;
			}
			break;

		case IDOK:
		case IDC_FINDNEXT:
			{
				int bufSize = GetWindowTextLength(GetDlgItem(hwnd, IDC_SEARCHTEXT)) + 1;
				TCHAR *buf = new TCHAR[bufSize];
				GetWindowText(GetDlgItem(hwnd, IDC_SEARCHTEXT), buf, GetWindowTextLength(GetDlgItem(hwnd, IDC_SEARCHTEXT)) + 1);
				SendMessage(GetDlgItem(hwnd, IDC_ITEMS2), NSM_FINDNEXT, (WPARAM)buf, 0);
				delete[] buf;
			}
			break;

		case IDC_COPY:
			SendMessage(GetDlgItem(hwnd, IDC_ITEMS2), NSM_COPY, 0, 0);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

INT_PTR svcShowNewstory(WPARAM wParam, LPARAM)
{
	HWND hwnd = (HWND)WindowList_Find(hNewstoryWindows, (MCONTACT)wParam);
	if (hwnd && IsWindow(hwnd)) {
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetFocus(hwnd);
	}
	else if (AskPassword((MCONTACT)wParam)) {
		HWND hwnd2 = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HISTORY), 0, HistoryDlgProc, wParam);
		ShowWindow(hwnd2, SW_SHOWNORMAL);
	}
	return 0;
}

INT_PTR svcShowSystemNewstory(WPARAM, LPARAM)
{
	HWND hwnd = (HWND)WindowList_Find(hNewstoryWindows, 0);
	if (hwnd && IsWindow(hwnd)) {
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetFocus(hwnd);
	}
	else if (AskPassword(0)) {
		HWND hwnd2 = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HISTORY), 0, HistoryDlgProc, 0);
		ShowWindow(hwnd2, SW_SHOWNORMAL);
	}
	return 0;
}
