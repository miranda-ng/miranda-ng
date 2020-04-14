////////////////////////////////////////////////////////////////////////
// NewStory -- new history viewer for Miranda IM
// (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
// Visit http://miranda-im.org/ for details on Miranda Instant Messenger
////////////////////////////////////////////////////////////////////////

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

int evtEventEdited(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = WindowList_Find(hNewstoryWindows, (UINT_PTR)wParam);
	SendMessage(hwnd, UM_EDITEVENT, wParam, lParam);
	return 0;
}

void InitHistory()
{
	hNewstoryWindows = WindowList_Create();

	HookEvent(ME_DB_EVENT_ADDED, evtEventAdded);
	HookEvent(ME_DB_EVENT_DELETED, evtEventDeleted);
	HookEvent(ME_DB_EVENT_EDITED, evtEventEdited);
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

/////////////////////////////////////////////////////////////////////////////////////////
// Main history dialog

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
	TBTN_LOGOPTIONS,
	TBTN_COUNT
};

int tbtnSpacing[TBTN_COUNT] = { 0, 0, TBTN_SPACER, 0, 0, TBTN_SPACER, 0, -1, 0 };

struct InfoBarEvents
{
	HWND hwndIco, hwndIcoIn, hwndIcoOut;
	HWND hwndTxt, hwndTxtIn, hwndTxtOut;
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

class CHistoryDlg : public CDlgBase
{
	HMENU m_hMenu;
	WORD showFlags;
	bool gonnaRedraw;
	bool isContactHistory;
	MCONTACT m_hContact;
	int lastYear = -1, lastMonth = -1, lastDay = -1;
	HTREEITEM hLastYear = 0, hLastMonth = 0, hLastDay = 0;
	bool disableTimeTreeChange = false;

	// window flags
	DWORD m_dwOptions = 0;

	// toolbar buttons
	HWND m_hwndBtnToolbar[TBTN_COUNT];

	// main controls
	HWND m_hwndTimeTree;

	// searchbar
	HWND m_hwndBtnCloseSearch;
	// statusbar
	HWND m_hwndStatus;
	// filter bar
	HWND m_hwndChkDateFrom, m_hwndChkDateTo;
	HWND m_hwndDateFrom, m_hwndDateTo;
	InfoBarEvents ibMessages, ibFiles, ibUrls, ibTotal;

	void ShowHideControls()
	{
		int cmd = (m_dwOptions & WND_OPT_FILTERBAR) ? SW_SHOW : SW_HIDE;
		ShowWindow(ibMessages.hwndIco, cmd);
		ShowWindow(ibMessages.hwndIcoIn, cmd);
		ShowWindow(ibMessages.hwndIcoOut, cmd);
		ShowWindow(ibMessages.hwndTxt, cmd);
		ShowWindow(ibMessages.hwndTxtIn, cmd);
		ShowWindow(ibMessages.hwndTxtOut, cmd);
		ShowWindow(ibFiles.hwndIco, cmd);
		ShowWindow(ibFiles.hwndIcoIn, cmd);
		ShowWindow(ibFiles.hwndIcoOut, cmd);
		ShowWindow(ibFiles.hwndTxt, cmd);
		ShowWindow(ibFiles.hwndTxtIn, cmd);
		ShowWindow(ibFiles.hwndTxtOut, cmd);
		ShowWindow(ibUrls.hwndIco, cmd);
		ShowWindow(ibUrls.hwndIcoIn, cmd);
		ShowWindow(ibUrls.hwndIcoOut, cmd);
		ShowWindow(ibUrls.hwndTxt, cmd);
		ShowWindow(ibUrls.hwndTxtIn, cmd);
		ShowWindow(ibUrls.hwndTxtOut, cmd);
		ShowWindow(ibTotal.hwndIco, cmd);
		ShowWindow(ibTotal.hwndIcoIn, cmd);
		ShowWindow(ibTotal.hwndIcoOut, cmd);
		ShowWindow(ibTotal.hwndTxt, cmd);
		ShowWindow(ibTotal.hwndTxtIn, cmd);
		ShowWindow(ibTotal.hwndTxtOut, cmd);
		ShowWindow(m_hwndDateFrom, cmd);
		ShowWindow(m_hwndDateTo, cmd);
		ShowWindow(m_hwndChkDateFrom, cmd);
		ShowWindow(m_hwndChkDateTo, cmd);
		ShowWindow(GetDlgItem(m_hwnd, IDC_IB_SEPARATOR), cmd);

		cmd = (m_dwOptions & WND_OPT_SEARCHBAR) ? SW_SHOW : SW_HIDE;
		ShowWindow(m_hwndBtnCloseSearch, cmd);
		ShowWindow(btnFindNext.GetHwnd(), cmd);
		ShowWindow(btnFindPrev.GetHwnd(), cmd);
		ShowWindow(edtSearchText.GetHwnd(), cmd);
		if (cmd)
			SetFocus(edtSearchText.GetHwnd());
	}

	void LayoutHistoryWnd()
	{
		int i;
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		int x, y; // tmp vars
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;

		HDWP hDwp = BeginDeferWindowPos(50);

		// toolbar
		int hToolBar = TBTN_SIZE + WND_SPACING;
		x = WND_SPACING;
		int btnReverse = -1;
		for (i = 0; i < TBTN_COUNT; ++i) {
			hDwp = DeferWindowPos(hDwp, m_hwndBtnToolbar[i], 0,
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
			hDwp = DeferWindowPos(hDwp, m_hwndBtnToolbar[i], 0,
				x, WND_SPACING,
				TBTN_SIZE, TBTN_SIZE,
				SWP_NOZORDER);
			x -= TBTN_SIZE + tbtnSpacing[i - 1];
		}

		// infobar
		//	hDwp = DeferWindowPos(hDwp, hwndIcoProtocol, 0,
		//		w-100+WND_SPACING, WND_SPACING,
		//		16, 16,
		//		SWP_NOZORDER);
		//	hDwp = DeferWindowPos(hDwp, hwndTxtNickname, 0,
		//		w-100+WND_SPACING*2+16, WND_SPACING,
		//		100, 16,
		//		SWP_NOZORDER);
		//	hDwp = DeferWindowPos(hDwp, hwndTxtUID, 0,
		//		w-100+WND_SPACING*2+16, WND_SPACING*2+16,
		//		100, 16,
		//		SWP_NOZORDER);

		// filter bar
		int hFilterBar = 0;
		if (m_dwOptions & WND_OPT_FILTERBAR) {
			hFilterBar = WND_SPACING + (16 + WND_SPACING) * 3;
			LayoutFilterBar(hDwp, WND_SPACING + (WND_SPACING + 75) * 0, WND_SPACING * 2 + hToolBar, 75, &ibMessages);
			LayoutFilterBar(hDwp, WND_SPACING + (WND_SPACING + 75) * 1, WND_SPACING * 2 + hToolBar, 75, &ibFiles);
			LayoutFilterBar(hDwp, WND_SPACING + (WND_SPACING + 75) * 2, WND_SPACING * 2 + hToolBar, 75, &ibUrls);
			LayoutFilterBar(hDwp, WND_SPACING + (WND_SPACING + 75) * 3, WND_SPACING * 2 + hToolBar, 75, &ibTotal);

			GetWindowRect(m_hwndChkDateFrom, &rc);
			x = rc.right - rc.left;
			GetWindowRect(m_hwndDateFrom, &rc);
			y = hToolBar + WND_SPACING + (WND_SPACING + (16 + WND_SPACING) * 3 - (rc.bottom - rc.top) * 2 - WND_SPACING) / 2;
			hDwp = DeferWindowPos(hDwp, m_hwndChkDateFrom, 0,
				w - x - (rc.right - rc.left) - WND_SPACING * 2, y,
				x, rc.bottom - rc.top,
				SWP_NOZORDER);
			hDwp = DeferWindowPos(hDwp, m_hwndDateFrom, 0,
				w - (rc.right - rc.left) - WND_SPACING, y,
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOZORDER);

			hDwp = DeferWindowPos(hDwp, m_hwndChkDateTo, 0,
				w - x - (rc.right - rc.left) - WND_SPACING * 2, y + (rc.bottom - rc.top) + WND_SPACING,
				x, rc.bottom - rc.top,
				SWP_NOZORDER);
			hDwp = DeferWindowPos(hDwp, m_hwndDateTo, 0,
				w - (rc.right - rc.left) - WND_SPACING, y + (rc.bottom - rc.top) + WND_SPACING,
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOZORDER);

			hDwp = DeferWindowPos(hDwp, GetDlgItem(m_hwnd, IDC_IB_SEPARATOR), 0,
				WND_SPACING, hToolBar + WND_SPACING,
				w - WND_SPACING * 2, 2,
				SWP_NOZORDER);
		}

		// general
		GetWindowRect(m_hwndStatus, &rc);
		int hStatus = rc.bottom - rc.top;
		hDwp = DeferWindowPos(hDwp, m_hwndStatus, 0,
			0, h - hStatus,
			w, hStatus,
			SWP_NOZORDER);

		int hSearch = 0;
		if (m_dwOptions & WND_OPT_SEARCHBAR) {
			GetWindowRect(edtSearchText.GetHwnd(), &rc);
			hSearch = rc.bottom - rc.top;
			hDwp = DeferWindowPos(hDwp, m_hwndBtnCloseSearch, 0,
				WND_SPACING, h - hSearch - hStatus - WND_SPACING,
				TBTN_SIZE, hSearch, SWP_NOZORDER);
			hDwp = DeferWindowPos(hDwp, edtSearchText.GetHwnd(), 0,
				TBTN_SIZE + WND_SPACING * 2, h - hSearch - hStatus - WND_SPACING,
				w - WND_SPACING * 4 - TBTN_SIZE * 3, hSearch,
				SWP_NOZORDER);
			hDwp = DeferWindowPos(hDwp, btnFindPrev.GetHwnd(), 0,
				w - WND_SPACING - TBTN_SIZE * 2, h - hSearch - hStatus - WND_SPACING,
				TBTN_SIZE, hSearch,
				SWP_NOZORDER);
			hDwp = DeferWindowPos(hDwp, btnFindNext.GetHwnd(), 0,
				w - WND_SPACING - TBTN_SIZE * 1, h - hSearch - hStatus - WND_SPACING,
				TBTN_SIZE, hSearch,
				SWP_NOZORDER);
			hSearch += WND_SPACING;
		}

		hDwp = DeferWindowPos(hDwp, m_histControl.GetHwnd(), 0,
			WND_SPACING, hToolBar + hFilterBar + WND_SPACING,
			w - WND_SPACING * 2, h - WND_SPACING * 2 - hFilterBar - hToolBar - hSearch - hStatus,
			SWP_NOZORDER);

		EndDeferWindowPos(hDwp);
	}

	CCtrlBase m_histControl;
	CCtrlEdit edtSearchText;
	CCtrlMButton btnUserInfo, btnSendMsg, btnUserMenu, btnCopy, btnOptions, btnFilter;
	CCtrlMButton btnCalendar, btnSearch, btnExport, btnFindNext, btnFindPrev;
	CCtrlTreeView m_timeTree;

public:
	CHistoryDlg(MCONTACT _hContact) :
		CDlgBase(g_plugin, IDD_HISTORY),
		m_hContact(_hContact),
		m_timeTree(this, IDC_TIMETREE),
		m_histControl(this, IDC_ITEMS2),
		edtSearchText(this, IDC_SEARCHTEXT),
		btnCopy(this, IDC_COPY, g_plugin.getIcon(ICO_COPY), LPGEN("Copy")),
		btnExport(this, IDC_EXPORT, g_plugin.getIcon(ICO_EXPORT), LPGEN("Export...")),
		btnFilter(this, IDC_FILTER, g_plugin.getIcon(ICO_FILTER), LPGEN("Filter")),
		btnSearch(this, IDC_SEARCH, g_plugin.getIcon(ICO_SEARCH), LPGEN("Search...")),
		btnOptions(this, IDC_LOGOPTIONS, g_plugin.getIcon(ICO_OPTIONS), LPGEN("Options")),
		btnSendMsg(this, IDC_MESSAGE, g_plugin.getIcon(ICO_SENDMSG), LPGEN("Send message")),
		btnCalendar(this, IDC_DATEPOPUP, g_plugin.getIcon(ICO_CALENDAR), LPGEN("Jump to date")),
		btnUserInfo(this, IDC_USERINFO, g_plugin.getIcon(ICO_USERINFO), LPGEN("User info")),
		btnUserMenu(this, IDC_USERMENU, g_plugin.getIcon(ICO_USERMENU), LPGEN("User menu")),
		btnFindNext(this, IDC_FINDNEXT, g_plugin.getIcon(ICO_FINDNEXT), LPGEN("Find next")),
		btnFindPrev(this, IDC_FINDPREV, g_plugin.getIcon(ICO_FINDPREV), LPGEN("Find previous"))
	{
		m_timeTree.OnSelChanged = Callback(this, &CHistoryDlg::onSelChanged_TimeTree);
		
		edtSearchText.OnChange = Callback(this, &CHistoryDlg::onChange_SearchText);

		btnCopy.OnClick = Callback(this, &CHistoryDlg::onClick_Copy);
		btnExport.OnClick = Callback(this, &CHistoryDlg::onClick_Export);
		btnFilter.OnClick = Callback(this, &CHistoryDlg::onClick_Filter);
		btnSearch.OnClick = Callback(this, &CHistoryDlg::onClick_Search);
		btnOptions.OnClick = Callback(this, &CHistoryDlg::onClick_Options);
		btnSendMsg.OnClick = Callback(this, &CHistoryDlg::onClick_Message);
		btnCalendar.OnClick = Callback(this, &CHistoryDlg::onClick_Calendar);
		btnFindNext.OnClick = Callback(this, &CHistoryDlg::onClick_FindNext);
		btnFindPrev.OnClick = Callback(this, &CHistoryDlg::onClick_FindPrev);
		btnUserInfo.OnClick = Callback(this, &CHistoryDlg::onClick_UserInfo);
		btnUserMenu.OnClick = Callback(this, &CHistoryDlg::onClick_UserMenu);

		showFlags = g_plugin.getDword(m_hContact, "showFlags", 0x7f);

		m_hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_POPUPS));
		HMENU hMenu = GetSubMenu(m_hMenu, 1);
		CheckMenuItem(hMenu, ID_FILTER_INCOMING,
			showFlags & HIST_SHOW_IN ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_OUTGOING,
			showFlags & HIST_SHOW_OUT ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_MESSAGES,
			showFlags & HIST_SHOW_MSGS ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_FILES,
			showFlags & HIST_SHOW_FILES ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_URLS,
			showFlags & HIST_SHOW_URLS ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_STATUS,
			showFlags & HIST_SHOW_STATUS ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_OTHER,
			showFlags & HIST_SHOW_OTHER ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_AUTO,
			showFlags & HIST_AUTO_FILTER ? MF_CHECKED : MF_UNCHECKED);

		//			CheckMenuItem(hMenu, ID_LOGOPTIONS_SHOWTIMETREE,
		//				showFlags&HIST_TIMETREE ? MF_CHECKED : MF_UNCHECKED);
		//			ShowWindow(GetDlgItem(m_hwnd, IDC_TIMETREE), showFlags & HIST_TIMETREE ? SW_SHOW : SW_HIDE);
	}

	bool OnInitDialog() override
	{
		// get handles
		m_hwndBtnToolbar[TBTN_USERINFO] = btnUserInfo.GetHwnd();
		m_hwndBtnToolbar[TBTN_USERMENU] = btnUserMenu.GetHwnd();
		m_hwndBtnToolbar[TBTN_MESSAGE] = btnSendMsg.GetHwnd();
		m_hwndBtnToolbar[TBTN_SEARCH] = btnSearch.GetHwnd();
		m_hwndBtnToolbar[TBTN_COPY] = btnCopy.GetHwnd();
		m_hwndBtnToolbar[TBTN_EXPORT] = btnExport.GetHwnd();
		m_hwndBtnToolbar[TBTN_LOGOPTIONS] = btnOptions.GetHwnd();
		m_hwndBtnToolbar[TBTN_FILTER] = btnFilter.GetHwnd();
		m_hwndBtnToolbar[TBTN_DATEPOPUP] = btnCalendar.GetHwnd();

		m_hwndBtnCloseSearch = GetDlgItem(m_hwnd, IDC_SEARCHICON);
		m_hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, m_hwnd, NULL, g_plugin.getInst(), NULL);
		SendMessage(m_hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);

		// filterbar
		SendMessage(m_hwndBtnToolbar[TBTN_FILTER], BUTTONSETASPUSHBTN, 0, 0);
		SendMessage(m_hwndBtnToolbar[TBTN_SEARCH], BUTTONSETASPUSHBTN, 0, 0);

		m_hwndChkDateFrom = GetDlgItem(m_hwnd, IDC_CHK_DATE_FROM);
		m_hwndChkDateTo = GetDlgItem(m_hwnd, IDC_CHK_DATE_TO);
		m_hwndDateFrom = GetDlgItem(m_hwnd, IDC_DATE_FROM);
		m_hwndDateTo = GetDlgItem(m_hwnd, IDC_DATE_TO);

		ibMessages.hwndIco = GetDlgItem(m_hwnd, IDC_ICO_MESSAGES);
		ibMessages.hwndTxt = GetDlgItem(m_hwnd, IDC_TXT_MESSAGES);
		ibMessages.hwndIcoIn = GetDlgItem(m_hwnd, IDC_ICO_MESSAGES_IN);
		SendMessage(ibMessages.hwndIcoIn, BUTTONSETASFLATBTN, 0, 0);
		SendMessage(ibMessages.hwndIcoIn, BUTTONSETASPUSHBTN, 0, 0);
		SendMessage(ibMessages.hwndIcoIn, BM_SETCHECK, BST_CHECKED, 0);
		ibMessages.hwndTxtIn = GetDlgItem(m_hwnd, IDC_TXT_MESSAGES_IN);
		ibMessages.hwndIcoOut = GetDlgItem(m_hwnd, IDC_ICO_MESSAGES_OUT);
		SendMessage(ibMessages.hwndIcoOut, BUTTONSETASFLATBTN, 0, 0);
		SendMessage(ibMessages.hwndIcoOut, BUTTONSETASPUSHBTN, 0, 0);
		SendMessage(ibMessages.hwndIcoOut, BM_SETCHECK, BST_CHECKED, 0);
		ibMessages.hwndTxtOut = GetDlgItem(m_hwnd, IDC_TXT_MESSAGES_OUT);

		ibFiles.hwndIco = GetDlgItem(m_hwnd, IDC_ICO_FILES);
		ibFiles.hwndTxt = GetDlgItem(m_hwnd, IDC_TXT_FILES);
		ibFiles.hwndIcoIn = GetDlgItem(m_hwnd, IDC_ICO_FILES_IN);
		SendMessage(ibFiles.hwndIcoIn, BUTTONSETASFLATBTN, 0, 0);
		SendMessage(ibFiles.hwndIcoIn, BUTTONSETASPUSHBTN, 0, 0);
		SendMessage(ibFiles.hwndIcoIn, BM_SETCHECK, BST_CHECKED, 0);
		ibFiles.hwndTxtIn = GetDlgItem(m_hwnd, IDC_TXT_FILES_IN);
		ibFiles.hwndIcoOut = GetDlgItem(m_hwnd, IDC_ICO_FILES_OUT);
		SendMessage(ibFiles.hwndIcoOut, BUTTONSETASFLATBTN, 0, 0);
		SendMessage(ibFiles.hwndIcoOut, BUTTONSETASPUSHBTN, 0, 0);
		SendMessage(ibFiles.hwndIcoOut, BM_SETCHECK, BST_CHECKED, 0);
		ibFiles.hwndTxtOut = GetDlgItem(m_hwnd, IDC_TXT_FILES_OUT);

		ibUrls.hwndIco = GetDlgItem(m_hwnd, IDC_ICO_URLS);
		ibUrls.hwndTxt = GetDlgItem(m_hwnd, IDC_TXT_URLS);
		ibUrls.hwndIcoIn = GetDlgItem(m_hwnd, IDC_ICO_URLS_IN);
		SendMessage(ibUrls.hwndIcoIn, BUTTONSETASFLATBTN, 0, 0);
		SendMessage(ibUrls.hwndIcoIn, BUTTONSETASPUSHBTN, 0, 0);
		SendMessage(ibUrls.hwndIcoIn, BM_SETCHECK, BST_CHECKED, 0);
		ibUrls.hwndTxtIn = GetDlgItem(m_hwnd, IDC_TXT_URLS_IN);
		ibUrls.hwndIcoOut = GetDlgItem(m_hwnd, IDC_ICO_URLS_OUT);
		SendMessage(ibUrls.hwndIcoOut, BUTTONSETASFLATBTN, 0, 0);
		SendMessage(ibUrls.hwndIcoOut, BUTTONSETASPUSHBTN, 0, 0);
		SendMessage(ibUrls.hwndIcoOut, BM_SETCHECK, BST_CHECKED, 0);
		ibUrls.hwndTxtOut = GetDlgItem(m_hwnd, IDC_TXT_URLS_OUT);

		ibTotal.hwndIco = GetDlgItem(m_hwnd, IDC_ICO_TOTAL);
		ibTotal.hwndTxt = GetDlgItem(m_hwnd, IDC_TXT_TOTAL);
		ibTotal.hwndIcoIn = GetDlgItem(m_hwnd, IDC_ICO_TOTAL_IN);
		SendMessage(ibTotal.hwndIcoIn, BUTTONSETASFLATBTN, 0, 0);
		SendMessage(ibTotal.hwndIcoIn, BUTTONSETASPUSHBTN, 0, 0);
		SendMessage(ibTotal.hwndIcoIn, BM_SETCHECK, BST_CHECKED, 0);
		ibTotal.hwndTxtIn = GetDlgItem(m_hwnd, IDC_TXT_TOTAL_IN);
		ibTotal.hwndIcoOut = GetDlgItem(m_hwnd, IDC_ICO_TOTAL_OUT);
		SendMessage(ibTotal.hwndIcoOut, BUTTONSETASFLATBTN, 0, 0);
		SendMessage(ibTotal.hwndIcoOut, BUTTONSETASPUSHBTN, 0, 0);
		SendMessage(ibTotal.hwndIcoOut, BM_SETCHECK, BST_CHECKED, 0);
		ibTotal.hwndTxtOut = GetDlgItem(m_hwnd, IDC_TXT_TOTAL_OUT);

		// Ask for layout
		Utils_RestoreWindowPosition(m_hwnd, m_hContact, MODULENAME, "wnd_");
		PostMessage(m_hwnd, WM_SIZE, 0, 0);

		WindowList_Add(hNewstoryWindows, m_hwnd, m_hContact);

		if (m_hContact && (m_hContact != INVALID_CONTACT_ID)) {
			wchar_t *title = TplFormatString(TPL_TITLE, m_hContact, 0);
			SetWindowText(m_hwnd, title);
			mir_free(title);
		}
		else {
			if (m_hContact == INVALID_CONTACT_ID)
				SetWindowText(m_hwnd, TranslateT("History search results"));
			else
				SetWindowText(m_hwnd, TranslateT("System history"));
		}

		if (m_hContact != INVALID_CONTACT_ID)
			PostMessage(m_histControl.GetHwnd(), NSM_ADDHISTORY, m_hContact, 0);

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(ICO_NEWSTORY));

		SendMessage(GetDlgItem(m_hwnd, IDC_SEARCHICON), STM_SETICON, (WPARAM)g_plugin.getIcon(ICO_SEARCH), 0);

		SendMessage(ibMessages.hwndIco, STM_SETICON, (LPARAM)g_plugin.getIcon(ICO_SENDMSG), 0);
		SendMessage(ibMessages.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_MSGIN));
		SendMessage(ibMessages.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_MSGOUT));
		
		SendMessage(ibFiles.hwndIco, STM_SETICON, (LPARAM)g_plugin.getIcon(ICO_FILE), 0);
		SendMessage(ibFiles.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_MSGIN));
		SendMessage(ibFiles.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_MSGOUT));
		
		SendMessage(ibUrls.hwndIco, STM_SETICON, (LPARAM)g_plugin.getIcon(ICO_URL), 0);
		SendMessage(ibUrls.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_MSGIN));
		SendMessage(ibUrls.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_MSGOUT));
		
		SendMessage(ibTotal.hwndIco, STM_SETICON, (LPARAM)g_plugin.getIcon(ICO_UNKNOWN), 0);
		SendMessage(ibTotal.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_MSGIN));
		SendMessage(ibTotal.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_MSGOUT));

		SetFocus(m_histControl.GetHwnd());

		ShowHideControls();
		return true;
	}

	void OnDestroy() override
	{
		g_plugin.setDword(m_hContact, "showFlags", showFlags);
	
		Utils_SaveWindowPosition(m_hwnd, m_hContact, MODULENAME, "wnd_");
		Window_FreeIcon_IcoLib(m_hwnd);
		WindowList_Remove(hNewstoryWindows, m_hwnd);
		if (m_hwndStatus != nullptr) {
			DestroyWindow(m_hwndStatus);
			m_hwndStatus = nullptr;
		}

	}

	void onClick_Calendar(CCtrlButton *pButton)
	{
		RECT rc;
		GetWindowRect(pButton->GetHwnd(), &rc);

		time_t tm_jump = CalendarTool_Show(m_hwnd, rc.left, rc.bottom);
		if (tm_jump)
			m_histControl.SendMsg(NSM_SEEKTIME, tm_jump, 0);
	}

	void onClick_Copy(CCtrlButton *)
	{
		m_histControl.SendMsg(NSM_COPY, 0, 0);
	}

	void onClick_Export(CCtrlButton *)
	{
		// ExportHistoryDialog(m_hContact, m_hwnd);
		// DialogBox(hInst, MAKEINTRESOURCE(IDD_EXPORT), m_hwnd, ExportWndProc);
	}

	void onClick_Filter(CCtrlButton *)
	{
		if (m_dwOptions & WND_OPT_FILTERBAR)
			m_dwOptions &= ~WND_OPT_FILTERBAR;
		else
			m_dwOptions |= WND_OPT_FILTERBAR;

		ShowHideControls();
		LayoutHistoryWnd();
	}

	void onClick_FindNext(CCtrlButton *)
	{
		m_histControl.SendMsg(NSM_FINDNEXT, ptrW(edtSearchText.GetText()), 0);
	}

	void onClick_FindPrev(CCtrlButton *)
	{
		m_histControl.SendMsg(NSM_FINDPREV, ptrW(edtSearchText.GetText()), 0);
	}


	void onClick_Message(CCtrlButton *)
	{
		CallService(MS_MSG_SENDMESSAGE, m_hContact, 0);
	}

	void onClick_Options(CCtrlButton *pButton)
	{
		RECT rc;
		GetWindowRect(pButton->GetHwnd(), &rc);

		switch (TrackPopupMenu(GetSubMenu(m_hMenu, 2), TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, NULL)) {
		//	case ID_LOGOPTIONS_SHOWTIMETREE:
		//		showFlags = toggleBit(showFlags, HIST_TIMETREE);
		//		CheckMenuItem(GetSubMenu(hMenu, 1), ID_LOGOPTIONS_SHOWTIMETREE,
		//		showFlags&HIST_TIMETREE ? MF_CHECKED : MF_UNCHECKED);
		//		ShowWindow(GetDlgItem(m_hwnd, IDC_TIMETREE), showFlags&HIST_TIMETREE ? SW_SHOW : SW_HIDE);
		//		break;

		case ID_LOGOPTIONS_OPTIONS:
			g_plugin.openOptions(L"History", L"Newstory" /*, L"General" */);
			break;

		case ID_LOGOPTIONS_TEMPLATES:
			g_plugin.openOptions(L"History", L"Newstory" /* , L"Templates" */);
			break;
		}
		PostMessage(m_hwnd, WM_SIZE, 0, 0);
	}

	void onClick_Search(CCtrlButton *)
	{
		if (m_dwOptions & WND_OPT_SEARCHBAR)
			m_dwOptions &= ~WND_OPT_SEARCHBAR;
		else
			m_dwOptions |= WND_OPT_SEARCHBAR;

		ShowHideControls();
		LayoutHistoryWnd();
	}

	void onClick_UserInfo(CCtrlButton *)
	{
		CallService(MS_USERINFO_SHOWDIALOG, m_hContact, 0);
	}

	void onClick_UserMenu(CCtrlButton *pButton)
	{
		RECT rc;
		GetWindowRect(pButton->GetHwnd(), &rc);
	
		HMENU hMenu = Menu_BuildContactMenu(m_hContact);
		TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, m_hwnd, NULL);
		DestroyMenu(hMenu);
	}

	void onChange_SearchText(CCtrlEdit*)
	{
		if (showFlags & HIST_AUTO_FILTER)
			PostMessage(m_hwnd, UM_REBUILDLIST, 0, 0);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if ((msg >= NSM_FIRST) && (msg < NSM_LAST)) {
			LPARAM result = m_histControl.SendMsg(msg, wParam, lParam);
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, result);
			return result;
		}

		switch (msg) {
		case WM_MEASUREITEM:
			LPMEASUREITEMSTRUCT lpmis;
			lpmis = (LPMEASUREITEMSTRUCT)lParam;

			if (lpmis->CtlType == ODT_MENU)
				return Menu_MeasureItem(lParam);

			lpmis->itemHeight = 25;
			return TRUE;

		case WM_SIZE:
			LayoutHistoryWnd();
			return TRUE;

		case WM_CHARTOITEM:
			if (!((GetKeyState(VK_CONTROL) & 0x80) || (GetKeyState(VK_MENU) & 0x80))) {
				wchar_t s[] = { LOWORD(wParam), 0 };
				SetWindowText(GetDlgItem(m_hwnd, IDC_SEARCHTEXT), s);
				SendMessage(GetDlgItem(m_hwnd, IDC_SEARCHTEXT), EM_SETSEL, 1, 1);
				SetFocus(GetDlgItem(m_hwnd, IDC_SEARCHTEXT));
			}
			return -1;

		case WM_DRAWITEM:
			LPDRAWITEMSTRUCT lpdis;
			lpdis = (LPDRAWITEMSTRUCT)lParam;

			if (lpdis->CtlType == ODT_MENU)
				return Menu_DrawItem(lParam);

			return (lpdis->itemID != -1);

		case WM_COMMAND:
			if (Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, m_hContact))
				return true;
			break;

				/*
				GetWindowRect(GetDlgItem(m_hwnd, LOWORD(wParam)), &rc);
	//					DWORD itemID = 0;
				bool doFilter = true;
				switch (TrackPopupMenu(GetSubMenu(hMenu, 1), TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, NULL))
				{
					case ID_FILTER_INCOMING:
					{
						showFlags = toggleBit(showFlags, HIST_SHOW_IN);
						CheckMenuItem(GetSubMenu(hMenu, 1), ID_FILTER_INCOMING,
							showFlags&HIST_SHOW_IN ? MF_CHECKED : MF_UNCHECKED);
						break;
					}
					case ID_FILTER_OUTGOING:
					{
						showFlags = toggleBit(showFlags, HIST_SHOW_OUT);
						CheckMenuItem(GetSubMenu(hMenu, 1), ID_FILTER_OUTGOING,
							showFlags&HIST_SHOW_OUT ? MF_CHECKED : MF_UNCHECKED);
						break;
					}
					case ID_FILTER_MESSAGES:
					{
						showFlags = toggleBit(showFlags, HIST_SHOW_MSGS);
						CheckMenuItem(GetSubMenu(hMenu, 1), ID_FILTER_MESSAGES,
							showFlags&HIST_SHOW_MSGS ? MF_CHECKED : MF_UNCHECKED);
						break;
					}
					case ID_FILTER_FILES:
					{
						showFlags = toggleBit(showFlags, HIST_SHOW_FILES);
						CheckMenuItem(GetSubMenu(hMenu, 1), ID_FILTER_FILES,
							showFlags&HIST_SHOW_FILES ? MF_CHECKED : MF_UNCHECKED);
						break;
					}
					case ID_FILTER_URLS:
					{
						showFlags = toggleBit(showFlags, HIST_SHOW_URLS);
						CheckMenuItem(GetSubMenu(hMenu, 1), ID_FILTER_URLS,
							showFlags&HIST_SHOW_URLS ? MF_CHECKED : MF_UNCHECKED);
						break;
					}
					case ID_FILTER_STATUS:
					{
						showFlags = toggleBit(showFlags, HIST_SHOW_STATUS);
						CheckMenuItem(GetSubMenu(hMenu, 1), ID_FILTER_STATUS,
							showFlags&HIST_SHOW_STATUS ? MF_CHECKED : MF_UNCHECKED);
						break;
					}
					case ID_FILTER_OTHER:
					{
						showFlags = toggleBit(showFlags, HIST_SHOW_OTHER);
						CheckMenuItem(GetSubMenu(hMenu, 1), ID_FILTER_OTHER,
							showFlags&HIST_SHOW_OTHER ? MF_CHECKED : MF_UNCHECKED);
						break;
					}
					case ID_FILTER_AUTO:
					{
						showFlags = toggleBit(showFlags, HIST_AUTO_FILTER);
						CheckMenuItem(GetSubMenu(hMenu, 1), ID_FILTER_AUTO,
							showFlags&HIST_AUTO_FILTER ? MF_CHECKED : MF_UNCHECKED);
						break;
					}
					default:
					{
						doFilter = false;
						break;
					}
				}
				if (doFilter)
					PostMessage(m_hwnd, UM_REBUILDLIST, 0, 0);
				break;*/

				// case IDC_SEARCH:
				// int id = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SEARCH), 0, SearchDlgProc, (LPARAM)GetDlgItem(m_hwnd, IDC_ITEMS));
				// SendMessage(GetDlgItem(m_hwnd, IDC_ITEMS), LB_SETCARETINDEX, id, 0);
				// SendMessage(GetDlgItem(m_hwnd, IDC_ITEMS), LB_SETTOPINDEX, id, 0);
				// SendMessage(GetDlgItem(m_hwnd, IDC_ITEMS), LB_SELITEMRANGE, FALSE, MAKELPARAM(0,eventCount));
				// SendMessage(GetDlgItem(m_hwnd, IDC_ITEMS), LB_SELITEMRANGE, TRUE, MAKELPARAM(id,id));
				// break;
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onSelChanged_TimeTree(CCtrlTreeView::TEventInfo *)
	{
		if (disableTimeTreeChange) {
			disableTimeTreeChange = false;
		}
		else {
			// LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
			//	int id = pnmtv->itemNew.lParam;
			//	SendMessage(GetDlgItem(m_hwnd, IDC_ITEMS), LB_SETCARETINDEX, id, 0);
			//	SendMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_ITEMS, LBN_SELCHANGE), (LPARAM)GetDlgItem(m_hwnd, IDC_ITEMS));
			//	SendMessage(GetDlgItem(m_hwnd, IDC_ITEMS), LB_SETTOPINDEX, id, 0);
			//	SendMessage(GetDlgItem(m_hwnd, IDC_ITEMS), LB_SELITEMRANGE, FALSE, MAKELPARAM(0,eventCount));
			//	SendMessage(GetDlgItem(m_hwnd, IDC_ITEMS), LB_SELITEMRANGE, TRUE, MAKELPARAM(id,id));
		}
	}

	// case UM_REBUILDLIST:
	//			if (showFlags & HIST_TIMETREE)
	//				ShowWindow(GetDlgItem(m_hwnd, IDC_TIMETREE), SW_SHOW);
	//			ShowWindow(GetDlgItem(m_hwnd, IDC_ITEMS2), SW_SHOW);
	//			ShowWindow(GetDlgItem(m_hwnd, IDC_SEARCHICON), SW_SHOW);
};

INT_PTR svcShowNewstory(WPARAM hContact, LPARAM)
{
	HWND hwnd = (HWND)WindowList_Find(hNewstoryWindows, hContact);
	if (hwnd && IsWindow(hwnd)) {
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetFocus(hwnd);
	}
	else (new CHistoryDlg(hContact))->Show();

	return 0;
}

INT_PTR svcShowSystemNewstory(WPARAM, LPARAM)
{
	HWND hwnd = (HWND)WindowList_Find(hNewstoryWindows, 0);
	if (hwnd && IsWindow(hwnd)) {
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetFocus(hwnd);
	}
	else (new CHistoryDlg(0))->Show();

	return 0;
}
