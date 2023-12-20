/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static CMOption<int> g_splitter(MODULENAME, "SplitterY", 100);

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
	WND_OPT_TIMETREE  = 0x01,
	WND_OPT_SEARCHBAR = 0x02,
	WND_OPT_FILTERBAR = 0x04,
	WND_OPT_BOOKMARKS = 0x08,
};

enum
{
	WND_SPACING = 4,
	TBTN_SIZE = 25,
};

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
	DeferWindowPos(hDwp, ib->hwndTxtOut, 0,
		x + 32 + WND_SPACING, y + (16 + WND_SPACING) * 2, w - WND_SPACING - 32, 16, SWP_NOZORDER);
}

static const char *pSettings[] =
{
	LPGEN("FirstName"),
	LPGEN("LastName"),
	LPGEN("e-mail"),
	LPGEN("Nick"),
	LPGEN("Age"),
	LPGEN("Gender"),
	LPGEN("City"),
	LPGEN("State"),
	LPGEN("Phone"),
	LPGEN("Homepage"),
	LPGEN("About")
};

class CHistoryDlg : public CDlgBase
{
	HMENU m_hMenu;
	uint16_t showFlags;
	bool m_bSearchChanged = false;
	MCONTACT m_hContact;
	int lastYear = -1, lastMonth = -1, lastDay = -1;
	HTREEITEM hLastYear = 0, hLastMonth = 0, hLastDay = 0;
	bool disableTimeTreeChange = false;
	bool bAppendOnly;

	// window flags
	uint32_t m_dwOptions = 0;

	// toolbar buttons
	struct Button
	{
		enum { RIGHT = 1, SPACED = 2 };

		Button(CCtrlMButton &_1, int _2 = 0) :
			hwnd(_1),
			options(_2)
		{}

		CCtrlMButton &hwnd;
		int options;
	};
	std::vector<Button> m_toolbar;

	// main controls
	int m_iSplitter;

	CCtrlBase m_histWindow;
	NewstoryListData *m_histCtrl;

	// bookmarks
	HIMAGELIST hBookmarksIcons = 0;
	std::map<MCONTACT, std::vector<MEVENT>> m_events; // for filtered events

	// searchbar
	HWND m_hwndBtnCloseSearch;

	// statusbar
	HWND m_hwndStatus;

	// filter bar
	HWND m_hwndChkDateFrom, m_hwndChkDateTo;
	HWND m_hwndDateFrom, m_hwndDateTo;
	InfoBarEvents ibMessages, ibFiles, ibUrls, ibTotal;

	OBJLIST<SearchResult> m_arResults;

	static int stubSortResults(const void *p1, const void *p2)
	{
		return int((*(SearchResult **)p1)->ts) - int((*(SearchResult **)p2)->ts);
	}

	void DoGlobalSearch()
	{
		ptrW wszPattern(edtSearchText.GetText());
		if (!mir_wstrlen(wszPattern))
			return;

		// clear messages array first
		m_histCtrl->Clear();

		CharLowerW(wszPattern);
		DoSearchContact(0, wszPattern);
		for (auto &hContact : Contacts())
			DoSearchContact(hContact, wszPattern);

		qsort(m_arResults.getArray(), m_arResults.getCount(), sizeof(void *), stubSortResults);
		m_histCtrl->AddResults(m_arResults);
		m_arResults.destroy();

		BuildTimeTree();
	}

	void DoSearchContact(MCONTACT hContact, const wchar_t *pwszPattern)
	{
		DB::ECPTR pCursor(DB::Events(hContact));
		while (MEVENT hDbEvent = pCursor.FetchNext()) {
			DB::EventInfo dbei(hDbEvent);
			if (!dbei)
				continue;

			ptrW pwszText(DbEvent_GetTextW(&dbei, CP_UTF8));
			if (!mir_wstrlen(pwszText))
				continue;

			CharLowerW(pwszText);
			if (wcsstr(pwszText, pwszPattern))
				m_arResults.insert(new SearchResult(hContact, hDbEvent, dbei.timestamp));
		}
	}

	void ExportEvent(ItemData *pItem, FILE *out)
	{
		DB::EventInfo dbei(pItem->hEvent);
		if (!dbei)
			return;

		if (bAppendOnly) {
			fseek(out, -4, SEEK_END);
			fputs(",", out);
		}

		JSONNode pRoot2;
		pRoot2.push_back(JSONNode("type", dbei.eventType));

		char *szProto = Proto_GetBaseAccountName(pItem->hContact);
		if (mir_strcmp(dbei.szModule, szProto))
			pRoot2.push_back(JSONNode("module", dbei.szModule));

		pRoot2.push_back(JSONNode("timestamp", dbei.timestamp));

		wchar_t szTemp[500];
		TimeZone_PrintTimeStamp(UTC_TIME_HANDLE, dbei.timestamp, L"I", szTemp, _countof(szTemp), 0);
		pRoot2.push_back(JSONNode("isotime", T2Utf(szTemp).get()));

		std::string flags;
		if (dbei.flags & DBEF_SENT)
			flags += "m";
		if (dbei.flags & DBEF_READ)
			flags += "r";
		pRoot2.push_back(JSONNode("flags", flags));

		ptrW msg(DbEvent_GetTextW(&dbei, CP_ACP));
		if (msg)
			pRoot2.push_back(JSONNode("body", T2Utf(msg).get()));

		fputs(pRoot2.write_formatted().c_str(), out);
		fputs("\n]}", out);

		bAppendOnly = true;
	}

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

		bool bShowTime = (m_dwOptions & WND_OPT_TIMETREE) != 0;
		bool bShowBookmarks = (m_dwOptions & WND_OPT_BOOKMARKS) != 0;
		m_timeTree.Show(bShowTime);
		m_bookmarks.Show(bShowBookmarks);
		splitTime.Show(bShowTime || bShowBookmarks);
	}

	void UpdateTitle()
	{
		switch (m_hContact) {
		case INVALID_CONTACT_ID:
			SetWindowText(m_hwnd, TranslateT("Global history search"));
			break;

		case 0:
			SetWindowText(m_hwnd, TranslateT("System history"));
			break;

		default:
			SetWindowText(m_hwnd, TplFormatString(TPL_TITLE, m_hContact, 0));
		}
	}

	void BuildBookmarksList()
	{
		if (!(m_dwOptions & WND_OPT_BOOKMARKS))
			return;

		m_events.clear();
		m_bookmarks.DeleteAllItems();
		if (hBookmarksIcons)
			ImageList_Destroy(hBookmarksIcons);

		hBookmarksIcons = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 1, 0);
		m_bookmarks.SetImageList(hBookmarksIcons, LVSIL_SMALL);

		if (m_hContact == INVALID_CONTACT_ID) {
			auto *pDB = db_get_current();

			int iIconId = ImageList_ReplaceIcon(hBookmarksIcons, -1, g_plugin.getIcon(IDI_USERS));
			m_bookmarks.AddItem(TranslateT("All results"), iIconId, INVALID_CONTACT_ID);

			std::map<char *, int> protoMap;

			for (auto &cc : Contacts()) {
				bool bAdded = false;
				DB::ECPTR pCursor(pDB->EventCursor(cc, 0));
				while (MEVENT hEvent = pCursor.FetchNext()) {
					DB::EventInfo dbei(hEvent, false);
					if (dbei && dbei.flags & DBEF_BOOKMARK) {
						if (!bAdded) {
							auto *pa = Proto_GetContactAccount(cc);
							char *szProto = (pa == nullptr) ? "SomeShit" : pa->szModuleName;
							auto pProto = protoMap.find(szProto);

							if (pProto == protoMap.end()) {
								iIconId = ImageList_ReplaceIcon(hBookmarksIcons, -1, Skin_LoadProtoIcon(szProto, ID_STATUS_ONLINE));
								protoMap[szProto] = iIconId;
							}
							else iIconId = (*pProto).second;

							m_bookmarks.AddItem(Clist_GetContactDisplayName(cc, 0), iIconId, cc);
							bAdded = true;
						}

						m_events[cc].push_back(hEvent);
					}
				}
			}
		}
		else {
			ImageList_ReplaceIcon(hBookmarksIcons, -1, g_plugin.getIcon(IDI_BOOKMARK));

			int numItems = m_histCtrl->totalCount;

			for (int i = 0; i < numItems; i++) {
				auto *pItem = m_histCtrl->GetItem(i);
				if (pItem->fetch())
					if (pItem->dbe.flags & DBEF_BOOKMARK)
						m_bookmarks.AddItem(pItem->wtext, 0, i);
			}
		}
	}

	void BuildTimeTree()
	{
		if (!(m_dwOptions & WND_OPT_TIMETREE))
			return;

		m_timeTree.DeleteAllItems();
		auto &pArray = m_histCtrl->items;
		int numItems = pArray.getCount();

		int CurYear = 0, CurMonth = 0, CurDay = 0, PrevYear = -1, PrevMonth = -1, PrevDay = -1;
		HTREEITEM hCurYear = 0, hCurMonth = 0, hCurDay = 0;
		for (int i = 0; i < numItems; i++) {
			auto *pItem = pArray.get(i, false);
			if (!pItem->fetch())
				continue;

			if (pItem->dbe.timestamp == 0)
				continue;

			struct tm ts = { 0 };
			time_t timestamp = pItem->dbe.timestamp;
			errno_t err = localtime_s(&ts, &timestamp);  /* statically alloced, local time correction */
			if (err != 0)
				return;

			CurYear = ts.tm_year + 1900;
			CurMonth = ts.tm_mon + 1;
			CurDay = ts.tm_mday;
			wchar_t buf[50];
			TVINSERTSTRUCT tvi;
			tvi.hParent = nullptr;
			tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
			if (CurYear != PrevYear) {
				_itow(CurYear, buf, 10);
				tvi.item.pszText = buf;
				tvi.item.lParam = 0;
				hCurYear = TreeView_InsertItem(m_timeTree.GetHwnd(), &tvi);
				PrevYear = CurYear;
			}
			if (CurMonth != PrevMonth) {
				tvi.hParent = hCurYear;
				tvi.item.pszText = TranslateW(months[CurMonth - 1]);
				tvi.item.lParam = CurMonth;
				hCurMonth = TreeView_InsertItem(m_timeTree.GetHwnd(), &tvi);
				PrevMonth = CurMonth;
			}
			if (CurDay != PrevDay) {
				_itow(CurDay, buf, 10);
				tvi.hParent = hCurMonth;
				tvi.item.pszText = buf;
				tvi.item.lParam = 0;
				hCurDay = TreeView_InsertItem(m_timeTree.GetHwnd(), &tvi);
				PrevDay = CurDay;
			}
		}
		disableTimeTreeChange = true;
		HTREEITEM root = m_timeTree.GetRoot();
		m_timeTree.SelectItem(root);
	}

	CSplitter splitTime;
	CCtrlEdit edtSearchText;
	CCtrlMButton btnUserInfo, btnSendMsg, btnUserMenu, btnCopy, btnOptions, btnFilter;
	CCtrlMButton btnCalendar, btnSearch, btnExport, btnFindNext, btnFindPrev, btnDelete, btnTimeTree, btnBookmarks;
	CCtrlListView m_bookmarks;
	CCtrlTreeView m_timeTree;

public:
	CHistoryDlg(MCONTACT _hContact) :
		CDlgBase(g_plugin, IDD_HISTORY),
		m_arResults(10000),
		m_hContact(_hContact),
		m_iSplitter(g_splitter),
		m_bookmarks(this, IDC_BOOKMARKSLIST),
		m_timeTree(this, IDC_TIMETREEVIEW),
		m_histWindow(this, IDC_HISTORYCONTROL),
		splitTime(this, IDC_SPLITTERY),
		edtSearchText(this, IDC_SEARCHTEXT),
		btnCopy(this, IDC_COPY, g_plugin.getIcon(IDI_COPY), LPGEN("Copy")),
		btnExport(this, IDC_EXPORT, g_plugin.getIcon(IDI_EXPORT), LPGEN("Export...")),
		btnDelete(this, IDC_DELETE, SKINICON_OTHER_DELETE, LPGEN("Delete...")),
		btnFilter(this, IDC_FILTER, g_plugin.getIcon(IDI_FILTER), LPGEN("Filter")),
		btnSearch(this, IDC_SEARCH, g_plugin.getIcon(IDI_SEARCH), LPGEN("Search...")),
		btnOptions(this, IDC_LOGOPTIONS, g_plugin.getIcon(IDI_OPTIONS), LPGEN("Options")),
		btnSendMsg(this, IDC_MESSAGE, g_plugin.getIcon(IDI_SENDMSG), LPGEN("Send message")),
		btnCalendar(this, IDC_DATEPOPUP, g_plugin.getIcon(IDI_CALENDAR), LPGEN("Jump to date")),
		btnUserInfo(this, IDC_USERINFO, g_plugin.getIcon(IDI_USERINFO), LPGEN("User info")),
		btnUserMenu(this, IDC_USERMENU, g_plugin.getIcon(IDI_USERMENU), LPGEN("User menu")),
		btnFindNext(this, IDOK, g_plugin.getIcon(IDI_FINDNEXT), LPGEN("Find next")),
		btnFindPrev(this, IDC_FINDPREV, g_plugin.getIcon(IDI_FINDPREV), LPGEN("Find previous")),
		btnTimeTree(this, IDC_TIMETREE, g_plugin.getIcon(IDI_TIMETREE), LPGEN("Conversations")),
		btnBookmarks(this, IDC_BOOKMARKS, g_plugin.getIcon(IDI_BOOKMARK), LPGEN("Bookmarks"))
	{
		showFlags = g_plugin.getWord("showFlags", 0x7f);
		m_dwOptions = g_plugin.getDword("dwOptions");

		if (m_hContact == INVALID_CONTACT_ID)
			m_dwOptions |= WND_OPT_SEARCHBAR;
		else if (m_hContact > 0) {
			m_toolbar.push_back(Button(btnUserMenu));
			m_toolbar.push_back(Button(btnUserInfo));
			m_toolbar.push_back(Button(btnSendMsg, Button::SPACED));
		}

		m_toolbar.push_back(Button(btnTimeTree));
		m_toolbar.push_back(Button(btnBookmarks));
		m_toolbar.push_back(Button(btnSearch));
		m_toolbar.push_back(Button(btnFilter));
		m_toolbar.push_back(Button(btnCalendar, Button::SPACED));
		m_toolbar.push_back(Button(btnCopy));
		m_toolbar.push_back(Button(btnDelete));
		m_toolbar.push_back(Button(btnExport));
		m_toolbar.push_back(Button(btnOptions, Button::RIGHT));

		m_timeTree.OnSelChanged = Callback(this, &CHistoryDlg::onSelChanged_TimeTree);
		m_bookmarks.OnItemChanged = Callback(this, &CHistoryDlg::onSelChanged_Bookmarks);

		edtSearchText.OnChange = Callback(this, &CHistoryDlg::onChange_SearchText);

		splitTime.OnChange = Callback(this, &CHistoryDlg::onChange_Splitter);

		btnCopy.OnClick = Callback(this, &CHistoryDlg::onClick_Copy);
		btnExport.OnClick = Callback(this, &CHistoryDlg::onClick_Export);
		btnDelete.OnClick = Callback(this, &CHistoryDlg::onClick_Delete);
		btnFilter.OnClick = Callback(this, &CHistoryDlg::onClick_Filter);
		btnSearch.OnClick = Callback(this, &CHistoryDlg::onClick_Search);
		btnOptions.OnClick = Callback(this, &CHistoryDlg::onClick_Options);
		btnSendMsg.OnClick = Callback(this, &CHistoryDlg::onClick_Message);
		btnCalendar.OnClick = Callback(this, &CHistoryDlg::onClick_Calendar);
		btnFindPrev.OnClick = Callback(this, &CHistoryDlg::onClick_FindPrev);
		btnUserInfo.OnClick = Callback(this, &CHistoryDlg::onClick_UserInfo);
		btnUserMenu.OnClick = Callback(this, &CHistoryDlg::onClick_UserMenu);
		btnTimeTree.OnClick = Callback(this, &CHistoryDlg::onClick_TimeTree);
		btnBookmarks.OnClick = Callback(this, &CHistoryDlg::onClick_Bookmarks);

		m_hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_POPUPS));
		TranslateMenu(m_hMenu);

		HMENU hMenu = GetSubMenu(m_hMenu, 0);
		CheckMenuItem(hMenu, ID_FILTER_INCOMING, (showFlags & HIST_SHOW_IN) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_OUTGOING, (showFlags & HIST_SHOW_OUT) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_MESSAGES, (showFlags & HIST_SHOW_MSGS) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_FILES, (showFlags & HIST_SHOW_FILES) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_URLS, (showFlags & HIST_SHOW_URLS) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_STATUS, (showFlags & HIST_SHOW_STATUS) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_OTHER, (showFlags & HIST_SHOW_OTHER) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_FILTER_AUTO, (showFlags & HIST_AUTO_FILTER) ? MF_CHECKED : MF_UNCHECKED);
	}

	bool OnInitDialog() override
	{
		// get handles
		m_hwndBtnCloseSearch = GetDlgItem(m_hwnd, IDC_SEARCHICON);
		m_hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, m_hwnd, NULL, g_plugin.getInst(), NULL);
		SendMessage(m_hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);

		// bookmarks
		RECT rc;
		GetClientRect(m_bookmarks.GetHwnd(), &rc);
		m_bookmarks.AddColumn(0, TranslateT("Bookmarks"), rc.right - rc.left);

		// filterbar
		btnFilter.MakePush();
		
		btnTimeTree.MakePush();
		if (m_dwOptions & WND_OPT_TIMETREE)
			btnTimeTree.Push(true);

		btnSearch.MakePush();
		if (m_dwOptions & WND_OPT_SEARCHBAR)
			btnSearch.Push(true);

		btnBookmarks.MakePush();
		if (m_dwOptions & WND_OPT_BOOKMARKS)
			btnBookmarks.Push(true);
		
		// filters
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
		PostMessage(m_hwnd, WM_SIZE, 0, 0);

		WindowList_Add(g_hNewstoryWindows, m_hwnd, m_hContact);

		ShowHideControls();
		UpdateTitle();

		m_histCtrl = (NewstoryListData *)GetWindowLongPtr(m_histWindow.GetHwnd(), 0);

		if (m_hContact == 0 || m_hContact == INVALID_CONTACT_ID) {
			btnSendMsg.Hide();
			btnUserInfo.Hide();
			btnUserMenu.Hide();
		}

		if (m_hContact != INVALID_CONTACT_ID) {
			Utils_RestoreWindowPosition(m_hwnd, m_hContact, MODULENAME, "wnd_");

			m_histCtrl->AddEvent(m_hContact, 0, -1);

			BuildTimeTree();
			SetFocus(m_histWindow.GetHwnd());
		}
		else Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "glb_");

		BuildBookmarksList();

		m_histCtrl->SetContact(m_hContact);
		m_histCtrl->ScrollBottom();

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_NEWSTORY));

		SendMessage(GetDlgItem(m_hwnd, IDC_SEARCHICON), STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_SEARCH), 0);

		SendMessage(ibMessages.hwndIco, STM_SETICON, (LPARAM)g_plugin.getIcon(IDI_SENDMSG), 0);
		SendMessage(ibMessages.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_MSGIN));
		SendMessage(ibMessages.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_MSGOUT));

		SendMessage(ibFiles.hwndIco, STM_SETICON, (LPARAM)Skin_LoadIcon(SKINICON_EVENT_FILE), 0);
		SendMessage(ibFiles.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_MSGIN));
		SendMessage(ibFiles.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_MSGOUT));

		SendMessage(ibUrls.hwndIco, STM_SETICON, (LPARAM)Skin_LoadIcon(SKINICON_EVENT_URL), 0);
		SendMessage(ibUrls.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_MSGIN));
		SendMessage(ibUrls.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_MSGOUT));

		SendMessage(ibTotal.hwndIco, STM_SETICON, (LPARAM)g_plugin.getIcon(IDI_UNKNOWN), 0);
		SendMessage(ibTotal.hwndIcoIn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_MSGIN));
		SendMessage(ibTotal.hwndIcoOut, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_MSGOUT));
		return true;
	}

	bool OnApply() override
	{
		if (m_bSearchChanged) {
			m_bSearchChanged = false;
			if (m_hContact == INVALID_CONTACT_ID)
				DoGlobalSearch();
		}

		int iOldCaret = m_histCtrl->caret;
		int res = m_histCtrl->FindNext(ptrW(edtSearchText.GetText()));
		if (res == -1)
			SetWindowTextW(m_hwndStatus, TranslateT("No more occuurences found"));
		else if (res < iOldCaret)
			SetWindowTextW(m_hwndStatus, TranslateT("Passed the end of history"));
		else
			SetWindowTextW(m_hwndStatus, L"");

		return false;
	}

	void OnResize() override
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		int y; // tmp vars
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;

		HDWP hDwp = BeginDeferWindowPos(43 + (int)m_toolbar.size());

		// toolbar
		int hToolBar = TBTN_SIZE + WND_SPACING;

		int x = WND_SPACING;
		for (auto &it : m_toolbar) {
			if (it.options & Button::RIGHT)
				continue;

			hDwp = DeferWindowPos(hDwp, it.hwnd.GetHwnd(), 0, x, WND_SPACING, TBTN_SIZE, TBTN_SIZE, SWP_NOZORDER);
			x += TBTN_SIZE;
			if (it.options & Button::SPACED)
				x += 10;
		}

		x = w - WND_SPACING - TBTN_SIZE;
		for (auto &it : m_toolbar) {
			if (it.options & Button::RIGHT) {
				hDwp = DeferWindowPos(hDwp, it.hwnd.GetHwnd(), 0, x, WND_SPACING, TBTN_SIZE, TBTN_SIZE, SWP_NOZORDER);
				x -= TBTN_SIZE;
			}
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
				SWP_NOZORDER | SWP_NOACTIVATE);
			hDwp = DeferWindowPos(hDwp, m_hwndDateFrom, 0,
				w - (rc.right - rc.left) - WND_SPACING, y,
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOZORDER | SWP_NOACTIVATE);

			hDwp = DeferWindowPos(hDwp, m_hwndChkDateTo, 0,
				w - x - (rc.right - rc.left) - WND_SPACING * 2, y + (rc.bottom - rc.top) + WND_SPACING,
				x, rc.bottom - rc.top,
				SWP_NOZORDER | SWP_NOACTIVATE);
			hDwp = DeferWindowPos(hDwp, m_hwndDateTo, 0,
				w - (rc.right - rc.left) - WND_SPACING, y + (rc.bottom - rc.top) + WND_SPACING,
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOZORDER | SWP_NOACTIVATE);

			hDwp = DeferWindowPos(hDwp, GetDlgItem(m_hwnd, IDC_IB_SEPARATOR), 0,
				WND_SPACING, hToolBar + WND_SPACING,
				w - WND_SPACING * 2, 2,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}

		// general
		GetWindowRect(m_hwndStatus, &rc);
		int hStatus = rc.bottom - rc.top;
		hDwp = DeferWindowPos(hDwp, m_hwndStatus, 0,
			0, h - hStatus,
			w, hStatus,
			SWP_NOZORDER | SWP_NOACTIVATE);

		// search bar
		int hSearch = 0;
		if (m_dwOptions & WND_OPT_SEARCHBAR) {
			GetWindowRect(edtSearchText.GetHwnd(), &rc);
			hSearch = rc.bottom - rc.top;
			hDwp = DeferWindowPos(hDwp, m_hwndBtnCloseSearch, 0,
				WND_SPACING, h - hSearch - hStatus - WND_SPACING,
				TBTN_SIZE, hSearch, SWP_NOZORDER | SWP_NOACTIVATE);
			hDwp = DeferWindowPos(hDwp, edtSearchText.GetHwnd(), 0,
				TBTN_SIZE + WND_SPACING * 2, h - hSearch - hStatus - WND_SPACING,
				w - WND_SPACING * 4 - TBTN_SIZE * 3, hSearch,
				SWP_NOZORDER | SWP_NOACTIVATE);
			hDwp = DeferWindowPos(hDwp, btnFindPrev.GetHwnd(), 0,
				w - WND_SPACING - TBTN_SIZE * 2, h - hSearch - hStatus - WND_SPACING,
				TBTN_SIZE, hSearch,
				SWP_NOZORDER | SWP_NOACTIVATE);
			hDwp = DeferWindowPos(hDwp, btnFindNext.GetHwnd(), 0,
				w - WND_SPACING - TBTN_SIZE * 1, h - hSearch - hStatus - WND_SPACING,
				TBTN_SIZE, hSearch,
				SWP_NOZORDER | SWP_NOACTIVATE);
			hSearch += WND_SPACING;
		}

		int iClientTop = WND_SPACING + hToolBar + hFilterBar;
		int iClientBottom = h - WND_SPACING - hSearch - hStatus - iClientTop;

		// time tree bar
		int hTimeTree = 0;
		bool bShowTime = (m_dwOptions & WND_OPT_TIMETREE) != 0;
		bool bShowBookmarks = (m_dwOptions & WND_OPT_BOOKMARKS) != 0;

		if (bShowTime || bShowBookmarks) {
			hTimeTree = m_iSplitter;
			hDwp = DeferWindowPos(hDwp, splitTime.GetHwnd(), 0, hTimeTree, iClientTop, WND_SPACING, iClientBottom, SWP_NOZORDER | SWP_NOACTIVATE);

			if (bShowTime && bShowBookmarks) {
				int hMiddle = iClientBottom / 2;
				hDwp = DeferWindowPos(hDwp, m_timeTree.GetHwnd(), 0, WND_SPACING, iClientTop, hTimeTree - WND_SPACING, hMiddle, SWP_NOZORDER | SWP_NOACTIVATE);
				hDwp = DeferWindowPos(hDwp, m_bookmarks.GetHwnd(), 0, WND_SPACING, iClientTop + hMiddle, hTimeTree - WND_SPACING, hMiddle, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else if (bShowTime)
				hDwp = DeferWindowPos(hDwp, m_timeTree.GetHwnd(), 0, WND_SPACING, iClientTop, hTimeTree - WND_SPACING, iClientBottom, SWP_NOZORDER | SWP_NOACTIVATE);
			else
				hDwp = DeferWindowPos(hDwp, m_bookmarks.GetHwnd(), 0, WND_SPACING, iClientTop, hTimeTree - WND_SPACING, iClientBottom, SWP_NOZORDER | SWP_NOACTIVATE);
		}

		hDwp = DeferWindowPos(hDwp, m_histWindow.GetHwnd(), 0,
			WND_SPACING + hTimeTree, iClientTop,
			w - WND_SPACING * 2 - hTimeTree, iClientBottom,
			SWP_NOZORDER | SWP_NOACTIVATE);

		EndDeferWindowPos(hDwp);
	}

	void OnDestroy() override
	{
		g_splitter = m_iSplitter;

		if (m_hContact != INVALID_CONTACT_ID)
			Utils_SaveWindowPosition(m_hwnd, m_hContact, MODULENAME, "wnd_");
		else
			Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "glb_");

		g_plugin.setWord(0, "showFlags", showFlags);
		g_plugin.setDword(0, "dwOptions", m_dwOptions);

		Window_FreeIcon_IcoLib(m_hwnd);
		WindowList_Remove(g_hNewstoryWindows, m_hwnd);

		if (m_hwndStatus != nullptr) {
			DestroyWindow(m_hwndStatus);
			m_hwndStatus = nullptr;
		}
	}

	void onClick_Calendar(CCtrlButton *pButton)
	{
		RECT rc;
		GetWindowRect(pButton->GetHwnd(), &rc);

		CalendarTool_Show(m_hwnd, rc.left, rc.bottom);
	}

	void onClick_Copy(CCtrlButton *)
	{
		m_histCtrl->Copy();
	}

	void onClick_Delete(CCtrlButton *)
	{
		if (!CallService(MS_HISTORY_EMPTY, m_hContact, 0)) {
			m_histCtrl->Clear();

			UpdateTitle();
			BuildTimeTree();
		}
	}

	void onClick_TimeTree(CCtrlButton *)
	{
		if (m_dwOptions & WND_OPT_TIMETREE)
			m_dwOptions &= ~WND_OPT_TIMETREE;
		else
			m_dwOptions |= WND_OPT_TIMETREE;

		OnResize();
		ShowHideControls();
		BuildTimeTree();
	}

	void onClick_Bookmarks(CCtrlButton *)
	{
		if (m_dwOptions & WND_OPT_BOOKMARKS)
			m_dwOptions &= ~WND_OPT_BOOKMARKS;
		else
			m_dwOptions |= WND_OPT_BOOKMARKS;

		OnResize();
		ShowHideControls();
		BuildBookmarksList();
	}

	void onClick_Export(CCtrlButton *)
	{
		wchar_t FileName[MAX_PATH];
		VARSW tszMirDir(L"%miranda_userdata%\\NewStoryExport");

		if (db_mc_isMeta(m_hContact)) {
			CMStringW SubContactsList, MessageText;
			bool FirstTime = true;
			int subcount = db_mc_getSubCount(m_hContact);
			for (int i = 0; i < subcount; i++) {
				MCONTACT hSubContact = db_mc_getSub(m_hContact, i);
				char *subproto = Proto_GetBaseAccountName(hSubContact);
				ptrW subid(Contact::GetInfo(CNF_UNIQUEID, hSubContact, subproto));
				if (FirstTime)
					SubContactsList.Append(subid);
				else
					SubContactsList.AppendFormat(L"\r\n%s", subid.get());
				FirstTime = false;
			}
			MessageText.AppendFormat(TranslateT("It is metacontact. For export use one of this subcontacts:\r\n%s"), SubContactsList.c_str());
			MessageBox(m_hwnd, MessageText, TranslateT("Export warning"), MB_OK | MB_ICONWARNING);
			return;
		}

		char *szProto = Proto_GetBaseAccountName(m_hContact);
		ptrW id(Contact::GetInfo(CNF_UNIQUEID, m_hContact, szProto));
		ptrW nick(Contact::GetInfo(CNF_DISPLAY, m_hContact, szProto));
		const char *uid = Proto_GetUniqueId(szProto);

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		CMStringW tszFilter, tszTitle, tszFileName;
		tszFilter.AppendFormat(L"%s (*.json)%c*.json%c%c", TranslateT("JSON files"), 0, 0, 0);
		tszTitle.AppendFormat(TranslateT("Export %s history"), nick);
		ofn.lpstrFilter = tszFilter;
		ofn.hwndOwner = nullptr;
		ofn.lpstrTitle = tszTitle;
		ofn.lpstrFile = FileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
		ofn.lpstrInitialDir = tszMirDir;
		*FileName = '\0';
		ofn.lpstrDefExt = L"";
		if (!GetSaveFileName(&ofn))
			return;

		// create file
		CreatePathToFileW(FileName);
		if (PathFileExistsW(FileName))
			DeleteFileW(FileName);

		FILE *out = _wfopen(FileName, L"wt");
		if (out == NULL)
			return;

		// export contact info
		JSONNode pRoot, pInfo, pHist(JSON_ARRAY);
		pInfo.set_name("info");
		if (szProto)
			pInfo.push_back(JSONNode("proto", szProto));

		if (id != NULL)
			pInfo.push_back(JSONNode(uid, T2Utf(id).get()));

		for (auto &it : pSettings) {
			wchar_t *szValue = db_get_wsa(m_hContact, szProto, it);
			if (szValue)
				pInfo.push_back(JSONNode(it, T2Utf(szValue).get()));
			mir_free(szValue);
		}

		pRoot.push_back(pInfo);

		pHist.set_name("history");
		pRoot.push_back(pHist);

		fputs(pRoot.write_formatted().c_str(), out);
		fseek(out, -4, SEEK_CUR);

		// export events
		int iDone = 0;
		bAppendOnly = false;
		auto &arItems = m_histCtrl->items;
		int iCount = arItems.getCount();
		for (int i = 0; i < iCount; i++) {
			auto *pItem = arItems.get(i);
			if (pItem->m_bSelected) {
				ExportEvent(pItem, out);
				iDone++;
			}
		}

		// no items selected? export whole history
		if (iDone == 0)
			for (int i = 0; i < iCount; i++)
				ExportEvent(arItems.get(i), out);

		// Close the file
		fclose(out);
		MessageBox(m_hwnd, TranslateT("Complete"), TranslateT("History export"), MB_OK | MB_ICONINFORMATION);
	}

	void onClick_Filter(CCtrlButton *)
	{
		if (m_dwOptions & WND_OPT_FILTERBAR)
			m_dwOptions &= ~WND_OPT_FILTERBAR;
		else
			m_dwOptions |= WND_OPT_FILTERBAR;

		ShowHideControls();
		OnResize();
	}

	void onClick_FindPrev(CCtrlButton *)
	{
		int iOldCaret = m_histCtrl->caret;
		int res = m_histCtrl->FindPrev(ptrW(edtSearchText.GetText()));
		if (res == -1)
			SetWindowTextW(m_hwndStatus, TranslateT("No more occuurences found"));
		else if (res > iOldCaret)
			SetWindowTextW(m_hwndStatus, TranslateT("Passed the beginning of history"));
		else
			SetWindowTextW(m_hwndStatus, L"");
	}

	void onClick_Message(CCtrlButton *)
	{
		CallService(MS_MSG_SENDMESSAGE, m_hContact, 0);
	}

	void onClick_Options(CCtrlButton *pButton)
	{
		RECT rc;
		GetWindowRect(pButton->GetHwnd(), &rc);

		switch (TrackPopupMenu(GetSubMenu(m_hMenu, 1), TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, NULL)) {
		case ID_LOGOPTIONS_OPTIONS:
			g_plugin.openOptions(L"History", L"NewStory", L"Advanced");
			break;

		case ID_LOGOPTIONS_TEMPLATES:
			g_plugin.openOptions(L"History", L"NewStory", L"Templates");
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
		OnResize();
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

	void onChange_SearchText(CCtrlEdit *)
	{
		m_bSearchChanged = true;

		if (m_bInitialized)
			if (showFlags & HIST_AUTO_FILTER)
				PostMessage(m_hwnd, UM_REBUILDLIST, 0, 0);
	}

	void onChange_Splitter(CSplitter *)
	{
		m_iSplitter = splitTime.GetPos();
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if ((msg >= NSM_FIRST) && (msg < NSM_LAST)) {
			LPARAM result = m_histWindow.SendMsg(msg, wParam, lParam);
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, result);
			return result;
		}

		MSG message = { m_hwnd, msg, wParam, lParam };
		switch (Hotkey_Check(&message, MODULENAME)) {
		case HOTKEY_SEEK_FORWARD:
			btnFindNext.Click();
			break;
		case HOTKEY_SEEK_BACK:
			btnFindPrev.Click();
			break;
		case HOTKEY_SEARCH:
			btnSearch.Click();
			break;
		}

		switch (msg) {
		case WM_CHARTOITEM:
			if (!((GetKeyState(VK_CONTROL) & 0x80) || (GetKeyState(VK_MENU) & 0x80))) {
				wchar_t s[] = { LOWORD(wParam), 0 };
				SetWindowText(GetDlgItem(m_hwnd, IDC_SEARCHTEXT), s);
				SendMessage(GetDlgItem(m_hwnd, IDC_SEARCHTEXT), EM_SETSEL, 1, 1);
				SetFocus(GetDlgItem(m_hwnd, IDC_SEARCHTEXT));
			}
			return -1;

		case WM_COMMAND:
			if (Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, m_hContact))
				return TRUE;
			break;

			/*
			GetWindowRect(GetDlgItem(m_hwnd, LOWORD(wParam)), &rc);
//					uint32_t itemID = 0;
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

		case UM_BOOKMARKS:
			BuildBookmarksList();
			break;

		case WM_USER + 0x600:
			if (wParam)
				m_histWindow.SendMsg(NSM_SEEKTIME, wParam, 0);
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onSelChanged_Bookmarks(CCtrlListView::TEventInfo *ev)
	{
		int idx = m_bookmarks.GetItemData(ev->nmlv->iItem);

		if (m_hContact == INVALID_CONTACT_ID) {
			m_histCtrl->Clear();

			if (idx == INVALID_CONTACT_ID) {
				for (auto &cc : m_events)
					for (auto &hEvent : cc.second)
						m_histCtrl->AddEvent(cc.first, hEvent, 1);
			}
			else for (auto &hEvent : m_events[idx])
				m_histCtrl->AddEvent(idx, hEvent, 1);
		}
		else m_histCtrl->SetPos(idx);
	}

	void onSelChanged_TimeTree(CCtrlTreeView::TEventInfo *)
	{
		wchar_t *val1, *val2, *val3;
		int yearsel = 0, monthsel = 0, daysel = 1;
		bool monthfound = false;
		if (disableTimeTreeChange)
			disableTimeTreeChange = false;
		else {
			HTREEITEM hti1 = m_timeTree.GetSelection();
			TVITEMEX tvi = { 0 };
			tvi.hItem = hti1;
			tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
			tvi.cchTextMax = MAX_PATH;
			tvi.lParam = 0;
			tvi.pszText = (wchar_t *)_alloca(MAX_PATH * sizeof(wchar_t));

			m_timeTree.GetItem(&tvi);
			val1 = tvi.pszText;
			if (tvi.lParam) {
				monthsel = tvi.lParam;
				monthfound = true;
			}
			HTREEITEM hti2 = m_timeTree.GetParent(hti1);
			if ((!monthfound) && (!hti2))
				yearsel = _wtoi(val1);
			if ((!monthfound) && (hti2))
				daysel = _wtoi(val1);
			if (hti2) {
				tvi.hItem = hti2;
				tvi.lParam = 0;
				m_timeTree.GetItem(&tvi);
				val2 = tvi.pszText;
				if (tvi.lParam) {
					monthsel = tvi.lParam;
					monthfound = true;
				}
				else
					yearsel = _wtoi(val2);
				HTREEITEM hti3 = m_timeTree.GetParent(hti2);
				if (hti3) {
					tvi.hItem = hti3;
					tvi.lParam = 0;
					m_timeTree.GetItem(&tvi);
					val3 = tvi.pszText;
					yearsel = _wtoi(val3);
				}
			}
			struct tm tm_sel;
			tm_sel.tm_hour = tm_sel.tm_min = tm_sel.tm_sec = 0;
			tm_sel.tm_isdst = 1;
			tm_sel.tm_mday = daysel;
			if (monthsel)
				tm_sel.tm_mon = monthsel - 1;
			else
				tm_sel.tm_mon = 0;
			tm_sel.tm_year = yearsel - 1900;
			PostMessage(m_hwnd, WM_USER + 0x600, mktime(&tm_sel), 0);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Services

INT_PTR svcShowNewstory(WPARAM hContact, LPARAM)
{
	HWND hwnd = (HWND)WindowList_Find(g_hNewstoryWindows, hContact);
	if (!hwnd) {
		auto *pDlg = new CHistoryDlg(hContact);
		pDlg->Show();
		hwnd = pDlg->GetHwnd();
	}

	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	return 0;
}

INT_PTR svcGlobalSearch(WPARAM, LPARAM)
{
	return svcShowNewstory(INVALID_CONTACT_ID, 0);
}
