/*
IEView history viewer plugin for Miranda IM

Copyright © 2005-2006 Cristian Libotean

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
#include "dlgHandlers.h"

//#define GAP_SIZE 2
#define GAP_SIZE 0
#define MIN_HISTORY_WIDTH 350
#define MIN_HISTORY_HEIGHT 100

static int statusHeight = 0;

struct WorkerThreadData{
	HWND hWnd;
	HistoryWindowData *data;
	IEVIEWEVENT ieEvent;
};

void LoadName(HWND hWnd);
int CalcIEViewPos(IEVIEWWINDOW *ieWnd, HWND hMainWindow);
int LoadIEView(HWND hWnd);
int DestroyIEView(HWND hWnd);
int LoadEvents(HWND hWnd);
int LoadPage(HWND hWnd, MEVENT hFirstEvent, long index, long shiftCount, long readCount, int direction);
int LoadNext(HWND hWnd);
int LoadPrev(HWND hWnd);
int ScrollToBottom(HWND hWnd);

void RefreshButtonStates(HWND hWnd);

int CalcIEViewPos(IEVIEWWINDOW *ieWnd, HWND hMainWindow)
{
	RECT rect;
	GetWindowRect(GetDlgItem(hMainWindow, IDC_IEVIEW_PLACEHOLDER), &rect);
	rect.right -= rect.left; rect.bottom -= rect.top;
	ScreenToClient(hMainWindow, (POINT*)&rect);

	/// @todo : find out why -1/+1 is required... or why IEView uses a border...
	ieWnd->x = -1 + rect.left + GAP_SIZE;
	ieWnd->y = -1 + rect.top + GAP_SIZE;
	ieWnd->cx = 2 + rect.right - (2 * GAP_SIZE);
	ieWnd->cy = 2 + rect.bottom - (2 * GAP_SIZE);
	return 0;
}

void LoadName(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);
	if (!data->contact) {
		SetWindowText(hWnd, TranslateT("System History"));
		return;
	}

	wchar_t buffer[1024];
	mir_snwprintf(buffer, L"'%s' - IEHistory", ptrW(Contact::GetInfo(CNF_DISPLAY, data->contact)).get());
	SetWindowText(hWnd, buffer);
}

int LoadIEView(HWND hWnd)
{
	IEVIEWWINDOW ieWnd = { sizeof(ieWnd) };
	ieWnd.iType = IEW_CREATE;
	ieWnd.dwMode = IEWM_HISTORY;
	ieWnd.dwFlags = 0;
	ieWnd.parent = hWnd;
	CalcIEViewPos(&ieWnd, hWnd);

	CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWnd);
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);
	data->hIEView = ieWnd.hwnd;
	return 0;
}

int MoveIeView(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);
	if (data) {
		IEVIEWWINDOW ieWnd = {};
		ieWnd.parent = hWnd;
		ieWnd.hwnd = data->hIEView;
		ieWnd.iType = IEW_SETPOS;
		CalcIEViewPos(&ieWnd, hWnd);
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWnd);
	}
	return 0;
}

int DestroyIEView(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);
	IEVIEWWINDOW ieWnd = {};
	ieWnd.parent = hWnd;
	ieWnd.hwnd = data->hIEView;
	ieWnd.iType = IEW_DESTROY;
	CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWnd);
	return 0;
}

void FillIEViewInfo(IEVIEWEVENTDATA *fillData, DBEVENTINFO dbInfo, uint8_t *blob)
{
	switch (dbInfo.eventType) {
	case EVENTTYPE_MESSAGE:
		fillData->iType = IEED_EVENT_MESSAGE;
		break;
	case EVENTTYPE_STATUS:
		fillData->iType = IEED_EVENT_STATUSCHANGE;
		break;
	case EVENTTYPE_FILE:
		fillData->iType = IEED_EVENT_FILE;
		break;
	}

	fillData->szNick.a = "<nick here>";
	fillData->bIsMe = (dbInfo.flags & DBEF_SENT);
	fillData->dwFlags = (dbInfo.flags & DBEF_SENT) ? IEEDF_SENT : 0;
	fillData->time = dbInfo.timestamp;
	size_t len = mir_strlen((char *)blob) + 1;
	uint8_t *pos;

	fillData->szText.a = (char *)blob;
	if (len < dbInfo.cbBlob) {
		pos = blob + len;
		fillData->szText.w = (wchar_t *)pos;
		fillData->dwFlags |= IEEDF_UNICODE_TEXT;
	}
}

uint32_t WINAPI WorkerThread(LPVOID lpvData)
{
	Log("%s", "Inside worker thread ...");
	WorkerThreadData *data = (WorkerThreadData *)lpvData;
	EnableWindow(GetDlgItem(data->hWnd, IDC_CLOSE), FALSE);
	const int LOAD_COUNT = 10;
	int count = 0;
	int target = data->ieEvent.count;
	int cLoad = LOAD_COUNT;
	int i;
	IEVIEWEVENTDATA ieData[LOAD_COUNT] = { 0 };
	uint8_t *messages[LOAD_COUNT] = {};
	MEVENT dbEvent = data->ieEvent.hDbEventFirst;
	for (i = 0; i < LOAD_COUNT; i++)
		ieData[i].next = &ieData[i + 1]; //it's a vector, so v[i]'s next element is v[i + 1]

	ieData[LOAD_COUNT - 1].next = nullptr;
	IEVIEWEVENT ieEvent = data->ieEvent;
	ieEvent.iType = IEE_LOG_MEM_EVENTS;
	ieEvent.eventData = ieData;
	DBEVENTINFO dbInfo = {};
	uint8_t *buffer = nullptr;
	int newSize, oldSize = 0;
	while (count < target) {
		cLoad = (count + LOAD_COUNT > target) ? target - count : LOAD_COUNT;
		ieEvent.count = -1;

		for (i = 0; i < cLoad; i++) {
			newSize = db_event_getBlobSize(dbEvent);
			if (newSize > oldSize) {
				buffer = (uint8_t*)realloc(buffer, newSize);
				dbInfo.pBlob = buffer;
				oldSize = newSize;
			}
			messages[i] = (uint8_t*)realloc(messages[i], newSize);
			dbInfo.cbBlob = newSize;
			if (!db_event_get(dbEvent, &dbInfo)) {
				memcpy(messages[i], dbInfo.pBlob, newSize);
				FillIEViewInfo(&ieData[i], dbInfo, messages[i]);
			}
			dbEvent = db_event_next(0, dbEvent);
		}
		ieData[cLoad - 1].next = nullptr; //cLoad < LOAD_COUNT will only happen once, at the end
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&ieEvent);
		count += cLoad;
	}
	for (i = 0; i < LOAD_COUNT; i++)
		free(messages[i]);

	free(buffer);
	EnableWindow(GetDlgItem(data->hWnd, IDC_CLOSE), TRUE);
	free(data);

	Log("%s", "WorkerThread finished ... returning");
	return 0;
}

int DoLoadEvents(HWND hWnd, HistoryWindowData *data, IEVIEWEVENT ieEvent)
{
	ieEvent.iType = IEE_CLEAR_LOG;
	CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&ieEvent);
	if (data->loadMethod == LOAD_IN_BACKGROUND) {
		WorkerThreadData *threadData = (WorkerThreadData *)malloc(sizeof(WorkerThreadData));
		threadData->data = data;
		threadData->hWnd = hWnd;
		threadData->ieEvent = ieEvent;
		WorkerThread(threadData);
	}
	else {
		ieEvent.iType = IEE_LOG_DB_EVENTS;
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&ieEvent);
		ScrollToBottom(hWnd);

		wchar_t buffer[256];
		_itow(data->index + 1, buffer, 10);
		SendDlgItemMessage(hWnd, IDC_STATUSBAR, SB_SETTEXT, 0 | SBT_POPOUT, (LPARAM)buffer);
		_itow(data->index + ieEvent.count, buffer, 10);
		SendDlgItemMessage(hWnd, IDC_STATUSBAR, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)buffer);
		_itow(data->count, buffer, 10);
		SendDlgItemMessage(hWnd, IDC_STATUSBAR, SB_SETTEXT, 3 | SBT_POPOUT, (LPARAM)buffer);
		RefreshButtonStates(hWnd);
	}
	return 0;
}

int LoadEvents(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);
	int count = db_event_count(data->contact);
	int bLastFirst = g_plugin.bShowLastFirst;
	int bRTL = g_plugin.bEnableRtl;
	bRTL = db_get_b(data->contact, "Tab_SRMsg", "RTL", bRTL);
	data->bEnableRTL = bRTL;
	data->count = count;
	if (data->itemsPerPage > count)
		data->itemsPerPage = count;

	IEVIEWEVENT ieEvent = { sizeof(ieEvent) };
	ieEvent.hwnd = data->hIEView;
	ieEvent.hContact = data->contact;
	ieEvent.count = (data->itemsPerPage <= 0) ? count : data->itemsPerPage;

	MEVENT hFirstEvent = db_event_first(data->contact);
	int num = 0;
	if ((data->itemsPerPage > 0) && (bLastFirst)) {
		num = data->count - data->itemsPerPage;
		hFirstEvent = GetNeededEvent(data->contact, hFirstEvent, num, DIRECTION_FORWARD);
	}
	data->index = num;

	data->hLastFirstEvent = hFirstEvent;
	ieEvent.hDbEventFirst = hFirstEvent;
	if (data->bEnableRTL)
		ieEvent.dwFlags |= IEEF_RTL;

	DoLoadEvents(hWnd, data, ieEvent);
	return 0;
}

int LoadPage(HWND hWnd, MEVENT hFirstEvent, long index, long shiftCount, long readCount, int direction)
{
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);
	int count = shiftCount;
	int newIndex = index;
	IEVIEWEVENT ieEvent = { sizeof(ieEvent) };
	ieEvent.hwnd = data->hIEView;
	ieEvent.hContact = data->contact;

	if (direction == DIRECTION_BACK) {
		newIndex -= shiftCount;
		if (newIndex < 0) {
			newIndex = 0;
			count = index;
		}
	}
	else {
		newIndex += shiftCount;
		if (newIndex + readCount > data->count) {
			count = data->count - newIndex;
			newIndex = data->count - readCount;
		}
	}
	data->index = newIndex;
	MEVENT hEvent = GetNeededEvent(data->contact, hFirstEvent, count, direction);
	data->hLastFirstEvent = hEvent;
	ieEvent.hDbEventFirst = hEvent;
	ieEvent.count = readCount;
	if (data->bEnableRTL)
		ieEvent.dwFlags |= IEEF_RTL;

	DoLoadEvents(hWnd, data, ieEvent);
	return 0;
}

int LoadPrev(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);
	LoadPage(hWnd, data->hLastFirstEvent, data->index, data->itemsPerPage, data->itemsPerPage, DIRECTION_BACK);
	int finish = data->index <= 0;
	return finish;
}

int Resizer(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_CLOSE:
	case IDC_SEARCH:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_IEVIEW_PLACEHOLDER:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	}

	return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
}

int LoadNext(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);
	LoadPage(hWnd, data->hLastFirstEvent, data->index, data->itemsPerPage, data->itemsPerPage, DIRECTION_FORWARD);
	int finish = data->index + data->itemsPerPage >= data->count;
	return finish;
}

int ScrollToBottom(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);
	IEVIEWWINDOW ieWnd = {};
	ieWnd.iType = IEW_SCROLLBOTTOM;
	ieWnd.hwnd = data->hIEView;
	ieWnd.parent = hWnd;
	CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWnd);
	return 0;
}

void RefreshButtonStates(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);
	int bPrev = data->index > 0;
	int bNext = data->index + data->itemsPerPage < data->count;
	EnableWindow(GetDlgItem(hWnd, IDC_PREV), bPrev);
	EnableWindow(GetDlgItem(hWnd, IDC_NEXT), bNext);
}

INT_PTR CALLBACK HistoryDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HistoryWindowData *data = (HistoryWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);

	switch (msg) {
	case WM_INITDIALOG:
		Log("Inside WM_INITDIALOG ...");
		TranslateDialogDefault(hWnd);
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		{
			int bRTL = g_plugin.bEnableRtl;
			if (bRTL)
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_RTLREADING);

			HWND hStatusBar = CreateWindow(STATUSCLASSNAME, //class
				L"-", //title
				WS_CHILD | WS_VISIBLE | SBARS_TOOLTIPS | SBARS_SIZEGRIP, //style
				0, 0, //x, y
				0, 0, //width, height
				hWnd, //parent
				(HMENU)IDC_STATUSBAR, //menu
				g_plugin.getInst(), //instance
				nullptr); //lpParam
			int x;
			int widths[] = { x = 50, x += 50, x += 150, -1 };
			int count = sizeof(widths) / sizeof(widths[0]);
			SendMessage(hStatusBar, SB_SETPARTS, count, (LPARAM)widths);
			//SendMessage(hStatusBar, SB_SETTIPTEXT, 1, (LPARAM) TranslateT("First event shown in page"));
			//SendMessage(hStatusBar, SB_SETTIPTEXT, 2, (LPARAM) TranslateT("Last event shown in page"));
			SendMessage(hStatusBar, SB_SETTEXT, 2 | SBT_POPOUT, (LPARAM)TranslateT("Out of a total of"));

			RECT rc;
			GetWindowRect(hStatusBar, &rc);
			statusHeight = rc.bottom - rc.top;

		}
		return TRUE;

	case WM_SHOWWINDOW:
		Log("Inside WM_SHOWWINDOW ...");
		LoadName(hWnd);
		LoadIEView(hWnd);
		LoadEvents(hWnd);
		{
			bool bAll = (data->itemsPerPage <= 0) || (data->itemsPerPage >= data->count);
			int bLastFirst = g_plugin.bShowLastFirst;
			if (!bLastFirst) {
				EnableWindow(GetDlgItem(hWnd, IDC_PREV), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_NEXT), !bAll);
			}
			else {
				EnableWindow(GetDlgItem(hWnd, IDC_PREV), !bAll);
				EnableWindow(GetDlgItem(hWnd, IDC_NEXT), FALSE);
			}
		}
		break;

	case WM_DESTROY:
		DestroyIEView(hWnd);
		free(data);
		WindowList_Remove(hOpenWindowsList, hWnd);
		break;

	case WM_CLOSE:
		if (IsWindowEnabled(GetDlgItem(hWnd, IDC_CLOSE)))
			DestroyWindow(hWnd);
		else
			MessageBox(hWnd, TranslateT("You can't close the window now, wait for all events to load."), TranslateT("Error"), MB_OK | MB_ICONERROR);
		break;

	case WM_WINDOWPOSCHANGING:
		Utils_ResizeDialog(hWnd, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_HISTORY), &Resizer);
		MoveIeView(hWnd);
		{
			WINDOWPOS *wndPos = (WINDOWPOS *)lParam;
			MoveWindow(GetDlgItem(hWnd, IDC_STATUSBAR), wndPos->x, wndPos->y + wndPos->cy - statusHeight, wndPos->x + wndPos->cx, statusHeight, TRUE);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case IDC_PREV:
			LoadPrev(hWnd);
			break;

		case IDC_NEXT:
			LoadNext(hWnd);
			break;

		case IDC_SEARCH:
			HWND hSearch = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SEARCH), hWnd, SearchDlgProc);
			if (hSearch == nullptr) {
				char buffer[1024];
				sprintf_s(buffer, "Error #%d", GetLastError());
				MessageBoxA(nullptr, buffer, "Error", MB_OK);
			}
			SearchWindowData *searchData = (SearchWindowData *)malloc(sizeof(SearchWindowData));
			searchData->contact = data->contact;
			searchData->hHistoryWindow = hWnd;
			searchData->hLastFoundEvent = NULL;
			searchData->index = 0;
			SetWindowLongPtr(hSearch, DWLP_USER, (LONG_PTR)searchData);
			ShowWindow(hSearch, SW_SHOW);
			break;
		}
		break;
	}
	return 0;
}

INT_PTR CALLBACK SearchDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SearchWindowData *data = (SearchWindowData *)GetWindowLongPtr(hWnd, DWLP_USER);

	switch (msg) {
	case WM_INITDIALOG:
		if (MyEnableThemeDialogTexture)
			MyEnableThemeDialogTexture((HWND)hWnd, ETDT_ENABLETAB);

		TranslateDialogDefault(hWnd);
		{
			TCITEM tabItem = { 0 };
			tabItem.pszText = TranslateT("Text search");
			tabItem.mask = TCIF_TEXT;
			SendDlgItemMessage(hWnd, IDC_TABS, TCM_INSERTITEM, 0, (LPARAM)&tabItem);
			tabItem.pszText = TranslateT("Time search");
			SendDlgItemMessage(hWnd, IDC_TABS, TCM_INSERTITEM, 1, (LPARAM)&tabItem);
		}

		CheckDlgButton(hWnd, IDC_DIRECTION_DOWN, BST_CHECKED);
		return TRUE;

	case WM_SHOWWINDOW:
		SetFocus(GetDlgItem(hWnd, IDC_SEARCH_TEXT));
		break;

	case WM_DESTROY:
		free(data);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_SEARCH_DATE:
		case IDC_SEARCH_TIME:
			data->hLastFoundEvent = NULL; //start from top if changes occur
			break;

		case IDC_TABS:
			switch (((LPNMHDR)lParam)->code) {
			case TCN_SELCHANGE:
				int tab = SendDlgItemMessage(hWnd, IDC_TABS, TCM_GETCURSEL, 0, 0);
				ShowWindow(GetDlgItem(hWnd, IDC_SEARCH_DATE), (tab == 1) ? SW_SHOW : SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, IDC_SEARCH_TIME), (tab == 1) ? SW_SHOW : SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, IDC_SEARCH_TEXT), (tab == 0) ? SW_SHOW : SW_HIDE);
				SetDlgItemText(hWnd, IDC_SEARCH_TEXT_STATIC, ((tab == 0) ? TranslateT("Text:") : TranslateT("Time:")));
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SEARCH_TEXT:
			switch (HIWORD(wParam)) {
			case EN_CHANGE:
				data->hLastFoundEvent = NULL; //start from top if changes occur
				break;
			}
			break;

		case IDCANCEL:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case IDC_FIND_NEXT:
			const HistoryWindowData *histData = (HistoryWindowData *)GetWindowLongPtr(data->hHistoryWindow, DWLP_USER);
			int direction = IsDlgButtonChecked(hWnd, IDC_DIRECTION_UP) ? DIRECTION_BACK : DIRECTION_FORWARD;
			int tab = SendDlgItemMessage(hWnd, IDC_TABS, TCM_GETCURSEL, 0, 0);
			int type = (tab == 0) ? SEARCH_TEXT : SEARCH_TIME;
			SearchResult searchResult;
			if (data->hLastFoundEvent == NULL)
				data->index = (direction == DIRECTION_FORWARD) ? 0 : histData->count;
			else
				data->hLastFoundEvent = GetNeededEvent(data->contact, data->hLastFoundEvent, 1, direction);

			if (type == SEARCH_TEXT) { //text search
				wchar_t text[2048]; //TODO buffer overrun
				GetDlgItemText(hWnd, IDC_SEARCH_TEXT, text, _countof(text));
				searchResult = SearchHistory(data->contact, data->hLastFoundEvent, text, direction, type);
			}
			else { //time search
				TimeSearchData tsData = { 0 };
				SYSTEMTIME date = { 0 }, time = { 0 };
				int res = SendDlgItemMessage(hWnd, IDC_SEARCH_DATE, DTM_GETSYSTEMTIME, 0, (LPARAM)&date);
				tsData.flags = (res == GDT_VALID) ? TSDF_DATE_SET : 0;
				res = SendDlgItemMessage(hWnd, IDC_SEARCH_TIME, DTM_GETSYSTEMTIME, 0, (LPARAM)&time);
				if (res == GDT_VALID) {
					tsData.flags |= TSDF_TIME_SET;
					date.wHour = time.wHour;
					date.wMinute = time.wMinute;
					date.wSecond = time.wSecond;
					date.wMilliseconds = time.wMilliseconds;
				}
				tsData.time = date;
				searchResult = SearchHistory(data->contact, data->hLastFoundEvent, &tsData, direction, type);
			}

			if (searchResult.hEvent) {
				data->index += (direction == DIRECTION_BACK) ? -searchResult.index : searchResult.index;
				LoadPage(data->hHistoryWindow, searchResult.hEvent, data->index, 5, 10, DIRECTION_BACK);
			}
			else MessageBox(nullptr, TranslateT("Search finished. No more entries..."), TranslateT("Information"), MB_OK | MB_ICONINFORMATION);

			data->hLastFoundEvent = searchResult.hEvent;
			break;
		}
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// options

class COptionsDlg : public CDlgBase
{
	CCtrlEdit edtCount;
	CCtrlCheck chkLoadAll, chkLoadNumber, chkRtl, chkLastFirst, chkLoadBack;

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_HISTORY),
		edtCount(this, IDC_EVENTS_COUNT),
		chkRtl(this, IDC_ENABLE_RTL),
		chkLoadAll(this, IDC_LOAD_ALL),
		chkLoadBack(this, IDC_LOAD_BACKGROUND),
		chkLoadNumber(this, IDC_LOAD_NUMBER),
		chkLastFirst(this, IDC_SHOW_LAST_FIRST)
	{
		chkLoadAll.OnChange = chkLoadNumber.OnChange = Callback(this, &COptionsDlg::onChange_All);

		CreateLink(edtCount, g_plugin.iLoadCount);
		CreateLink(chkRtl, g_plugin.bEnableRtl);
		CreateLink(chkLoadBack, g_plugin.bUseWorker);
		CreateLink(chkLastFirst, g_plugin.bShowLastFirst);
	}

	bool OnInitDialog() override
	{
		chkLoadAll.SetState(g_plugin.iLoadCount == 0);
		chkLoadNumber.SetState(!chkLoadAll.GetState());
		return true;
	}

	bool OnApply() override
	{
		if (chkLoadAll.GetState())
			g_plugin.iLoadCount = 0;
		return true;
	}

	void onChange_All(CCtrlCheck *)
	{
		bool bEnabled = chkLoadNumber.GetState();
		edtCount.Enable(bEnabled);
		chkLastFirst.Enable(bEnabled);
	}
};

int OnOptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.szTitle.a = LPGEN("History");
	odp.pDialog = new COptionsDlg();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
