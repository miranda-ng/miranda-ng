#include "common.h"

//************ Some helper resize stuff ******************

#define MIN_HISTORY_WIDTH  540
#define MIN_HISTORY_HEIGHT 300

#define GAP_SIZE 2

#define ANCHOR_LEFT     0x000001
#define ANCHOR_RIGHT		0x000002
#define ANCHOR_TOP      0x000004
#define ANCHOR_BOTTOM   0x000008
#define ANCHOR_ALL      ANCHOR_LEFT | ANCHOR_RIGHT | ANCHOR_TOP | ANCHOR_BOTTOM

HWND hHistoryWindow = 0; //the history window
PopupHistoryList lstPopupHistory; //defined in main.cpp

const TCHAR *szHistoryColumns[] = {_T("Title"), _T("Message"), _T("Timestamp")}; //need to make sure that the string and size vectors have the same number of elements
const int cxHistoryColumns[] = {100, 450, 115};
const int cHistoryColumns = sizeof(szHistoryColumns) / sizeof(szHistoryColumns[0]);

struct PopupHistoryWindowData{
	HWND hIEView;
};

void ScreenToClient(HWND hWnd, LPRECT rect)
{
	POINT pt;
	int cx = rect->right - rect->left;
	int cy = rect->bottom - rect->top;
	pt.x = rect->left;
	pt.y = rect->top;
	ScreenToClient(hWnd, &pt);
	rect->left = pt.x;
	rect->top = pt.y;
	rect->right = pt.x + cx;
	rect->bottom = pt.y + cy;
}

RECT AnchorCalcPos(HWND window, const RECT *rParent, const WINDOWPOS *parentPos, int anchors)
{
	RECT rChild;
	RECT rTmp;

	GetWindowRect(window, &rChild);
	ScreenToClient(parentPos->hwnd, &rChild);

	int cx = rParent->right - rParent->left;
	int cy = rParent->bottom - rParent->top;
	if ((cx == parentPos->cx) && (cy == parentPos->cy))
		{
			return rChild;
		}
	if (parentPos->flags & SWP_NOSIZE)
		{
			return rChild;
		}

	rTmp.left = parentPos->x - rParent->left;
	rTmp.right = (parentPos->x + parentPos->cx) - rParent->right;
	rTmp.bottom = (parentPos->y + parentPos->cy) - rParent->bottom;
	rTmp.top = parentPos->y - rParent->top;
	
	cx = (rTmp.left) ? -rTmp.left : rTmp.right;
	cy = (rTmp.top) ? -rTmp.top : rTmp.bottom;
	
	rChild.right += cx;
	rChild.bottom += cy;
	//expanded the window accordingly, now we need to enforce the anchors
	if ((anchors & ANCHOR_LEFT) && (!(anchors & ANCHOR_RIGHT)))
		{
			rChild.right -= cx;
		}
	if ((anchors & ANCHOR_TOP) && (!(anchors & ANCHOR_BOTTOM)))
		{
			rChild.bottom -= cy;
		}
	if ((anchors & ANCHOR_RIGHT) && (!(anchors & ANCHOR_LEFT)))
		{
			rChild.left += cx;
		}
	if ((anchors & ANCHOR_BOTTOM) && (!(anchors & ANCHOR_TOP)))
		{
			rChild.top += cy;
		}
	return rChild;
}

void AnchorMoveWindow(HWND window, const WINDOWPOS *parentPos, int anchors)
{
	RECT rParent;
	RECT rChild;
	
	if (parentPos->flags & SWP_NOSIZE)
		{
			return;
		}
	GetWindowRect(parentPos->hwnd, &rParent);
	rChild = AnchorCalcPos(window, &rParent, parentPos, anchors);
	MoveWindow(window, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, FALSE);
}

void AddAnchorWindowToDeferList(HDWP &hdWnds, HWND window, RECT *rParent, WINDOWPOS *wndPos, int anchors)
{
	RECT rChild = AnchorCalcPos(window, rParent, wndPos, anchors);
	hdWnds = DeferWindowPos(hdWnds, window, HWND_NOTOPMOST, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, SWP_NOZORDER);
}
//************************************************************

//Stucture passed to list sort function
struct SortParams{
	HWND hList;
	int column;
};

static int lastColumn = -1; //last sort column

int MatchesFilterCS(const TCHAR *filter, const PopupHistoryData *popupItem) //case sensitive
{
	if (_tcslen(filter) <= 0)	{ return 1;	} //if no filter is set then the popup item matches the filter
	int match = 0;
	
	match = (_tcsstr(popupItem->messageT, filter)) ? 1 : match; //check message part
	
	if (!match) //check title part of no match has been found
	{
		match = (_tcsstr(popupItem->titleT, filter)) ? 1 : match;
	}
	
	if (!match) //if no match has been found yet try to match the timestamp
	{
		TCHAR buffer[1024];
		struct tm *myTime = localtime(&popupItem->timestamp);
		_tcsftime(buffer, 1024, _T("%c"), myTime);
		match = (_tcsstr(buffer, filter)) ? 1 : match;
	}
	
	return match;
}

__inline void ConvertCase(TCHAR *dest, const TCHAR *source, size_t size)
{
	_tcsncpy(dest, source, size);
	_tcslwr(dest);
}

int MatchesFilterCI(const TCHAR *filterS, const PopupHistoryData *popupItem)
{
	if (_tcslen(filterS) <= 0)	{ return 1;	} //if no filter is set then the popup item matches the filter
	int match = 0;
	const int BUFFER_SIZE = 1024;
	TCHAR buffer[BUFFER_SIZE];
	TCHAR filterI[BUFFER_SIZE];
	
	ConvertCase(filterI, filterS, BUFFER_SIZE);
	
	ConvertCase(buffer, popupItem->messageT, BUFFER_SIZE); //check message part
	match = (_tcsstr(buffer, filterI)) ? 1 : match;
	
	if (!match) // check title part of no match has been found
	{
		ConvertCase(buffer, popupItem->titleT, BUFFER_SIZE);
		match = (_tcsstr(buffer, filterI)) ? 1 : match;
	}
	
	if (!match) //if no match has been found yet try to match the timestamp
	{
		struct tm *myTime = localtime(&popupItem->timestamp);
		_tcsftime(buffer, 1024, _T("%c"), myTime);
		match = (_tcsstr(buffer, filterI)) ? 1 : match;
	}
	
	return match;
}

int CALLBACK PopupsCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam)
{
	SortParams params = *(SortParams *) myParam;
	const int MAX_SIZE = 512;
	TCHAR text1[MAX_SIZE];
	TCHAR text2[MAX_SIZE];
	int res;
	
	ListView_GetItemText(params.hList, (int) lParam1, params.column, text1, SIZEOF(text1));
	ListView_GetItemText(params.hList, (int) lParam2, params.column, text2, SIZEOF(text2));
	
	res = _tcsicmp(text1, text2);
	
	res = (params.column == lastColumn) ? -res : res; //do reverse search on second click on same column
	
	return res;
}



int CalcCustomControlPos(IEVIEWWINDOW *ieWnd, HWND hMainWindow)
{
	RECT rect;
	GetWindowRect(GetDlgItem(hMainWindow, IDC_LST_HISTORY), &rect);
	ScreenToClient(hMainWindow, &rect);
	//GetClientRect(hMainWindow, &rect);

	ieWnd->x = rect.left + GAP_SIZE;
	ieWnd->y = rect.top + GAP_SIZE;
	ieWnd->cx = rect.right - rect.left - (2 * GAP_SIZE);
	ieWnd->cy = rect.bottom - rect.top - (2 * GAP_SIZE);
	return 0;
}

void MoveCustomControl(HWND hWnd, int renderer)
{
	switch (renderer)
	{
		case RENDER_HISTORYPP:
		case RENDER_IEVIEW:
		{
			PopupHistoryWindowData *data = (PopupHistoryWindowData *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (data)
			{
					IEVIEWWINDOW ieWnd = {0};
					ieWnd.cbSize = sizeof(ieWnd);
					ieWnd.parent = hWnd;
					ieWnd.hwnd = data->hIEView;
					ieWnd.iType = IEW_SETPOS;
					CalcCustomControlPos(&ieWnd, hWnd);
					
					CallService((renderer == RENDER_HISTORYPP) ? MS_HPP_EG_WINDOW : MS_IEVIEW_WINDOW, 0, (LPARAM) &ieWnd);
			}
			
			break;
		}
	}
}

void LoadRenderer(HWND hWnd, int renderer)
{
	switch (renderer)
	{
		case RENDER_HISTORYPP:
		case RENDER_IEVIEW:
		{
			IEVIEWWINDOW ieWnd = {0};
			
			ieWnd.cbSize = sizeof(ieWnd);
			ieWnd.iType = IEW_CREATE;
			ieWnd.dwMode = IEWM_HISTORY;
			ieWnd.dwFlags = 0;
			ieWnd.parent = hWnd;
			CalcCustomControlPos(&ieWnd, hWnd);
			
			CallService((renderer == RENDER_HISTORYPP) ? MS_HPP_EG_WINDOW : MS_IEVIEW_WINDOW, 0, (LPARAM) &ieWnd); //create the IeView or H++ control.
			
			PopupHistoryWindowData *data = (PopupHistoryWindowData *) mir_alloc(sizeof(PopupHistoryWindowData)); //create custom control data
			data->hIEView = ieWnd.hwnd;
			ShowWindow(data->hIEView, SW_SHOW);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) data); //set it as the window's user data
			ShowWindow(GetDlgItem(hWnd, IDC_LST_HISTORY), SW_HIDE);
			//SetWindowPos(GetDlgItem(hWnd, IDC_LST_HISTORY), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			
			break;
		}
	}
}

void UnloadRenderer(HWND hWnd, int renderer)
{
	switch (renderer)
	{
		case RENDER_HISTORYPP:
		case RENDER_IEVIEW:
		{
			PopupHistoryWindowData *data = (PopupHistoryWindowData *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
			
			if (data)
			{
				IEVIEWWINDOW ieWnd = {0};
				ieWnd.cbSize = sizeof(ieWnd);
				ieWnd.parent = hWnd;
				ieWnd.hwnd = data->hIEView;
				ieWnd.iType = IEW_DESTROY;
				CallService((renderer == RENDER_HISTORYPP) ? MS_HPP_EG_WINDOW : MS_IEVIEW_WINDOW, 0, (LPARAM) &ieWnd);		
				
				mir_free(data);
			}
			
			break;
		}
	}
}

void DeleteOldEvents(HWND hWnd, int renderer)
{
	switch (renderer)
	{
		case RENDER_HISTORYPP:
		case RENDER_IEVIEW:
		{
			PopupHistoryWindowData *data = (PopupHistoryWindowData *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (data)
			{
				IEVIEWEVENT ieEvent = {0};
				ieEvent.cbSize = sizeof(IEVIEWEVENT);
				ieEvent.hwnd = data->hIEView;
				ieEvent.iType = IEE_CLEAR_LOG;
				
				CallService((renderer == RENDER_HISTORYPP) ? MS_HPP_EG_EVENT : MS_IEVIEW_EVENT, 0, (LPARAM) &ieEvent);
			}
			
			break;
		}
		
		case RENDER_DEFAULT:
		default:
		{
			ListView_DeleteAllItems(GetDlgItem(hWnd, IDC_LST_HISTORY));
			
			break;
		}
		
	}
}

typedef int (*SIG_MATCHESFILTER)(const TCHAR *filter, const PopupHistoryData *popupItem);
typedef void (*SIG_ADDEVENTS)(HWND hWnd, int renderer, TCHAR *filter, SIG_MATCHESFILTER MatchesFilter);

IEVIEWEVENTDATA *CreateAndFillEventData(PopupHistoryData *popupItem)
{
	IEVIEWEVENTDATA *eventData = (IEVIEWEVENTDATA *) mir_calloc(sizeof(IEVIEWEVENTDATA));
	if (eventData)
	{
		eventData->cbSize = sizeof(IEVIEWEVENTDATA);
		eventData->iType = IEED_EVENT_MESSAGE;

		eventData->dwFlags = IEEDF_UNICODE_NICK | IEEDF_UNICODE_TEXT | IEEDF_UNICODE_TEXT2;
		eventData->pszNickW = popupItem->titleT;
		eventData->pszTextW = popupItem->messageT;

		eventData->time = (DWORD) popupItem->timestamp;
		eventData->next = NULL;
	}
	
	return eventData;
}

void AddEventsCustomControl(HWND hWnd, int renderer, TCHAR *filter, SIG_MATCHESFILTER MatchesFilter)
{
	PopupHistoryWindowData *pwData = (PopupHistoryWindowData *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (pwData)
	{
		IEVIEWEVENT ieEvent = {0};
		ieEvent.cbSize = sizeof(IEVIEWEVENT);
		ieEvent.hwnd = pwData->hIEView;
		ieEvent.codepage = CP_ACP;
		ieEvent.iType = IEE_LOG_MEM_EVENTS;
		ieEvent.hContact = NULL;
		
		IEVIEWEVENTDATA *eventData = NULL;
		IEVIEWEVENTDATA *cED = NULL;
		IEVIEWEVENTDATA *prevED = NULL;
		
		int i;
		int count = 0;
		int size = lstPopupHistory.Count();
		PopupHistoryData *popupItem;
		
		for (i = 0; i < size; i++)
		{
			popupItem = lstPopupHistory.Get(i);
			if (MatchesFilter(filter, popupItem))
			{
				cED = CreateAndFillEventData(popupItem);
				if (cED)
				{
					count++;
					if (!eventData)
					{
						eventData = cED;
					}
					else{
						prevED->next = cED;
					}
					
					prevED = cED;
				}
			}
		}
		ieEvent.count = count;
		ieEvent.eventData = eventData;
		
		CallService((renderer == RENDER_HISTORYPP) ? MS_HPP_EG_EVENT : MS_IEVIEW_EVENT, 0, (LPARAM) &ieEvent);
		
		while (eventData)
		{
			cED = eventData;
			eventData = eventData->next;
			mir_free(cED);
		}
	}
}

void AddEventsDefault(HWND hWnd, int renderer, TCHAR *filter, SIG_MATCHESFILTER MatchesFilter)
{
	HWND hHistoryList = GetDlgItem(hWnd, IDC_LST_HISTORY);
	TCHAR buffer[1024];
	struct tm *myTime;
	
	LVITEM item = {0};
	item.mask = LVIF_TEXT;

	int i, lIndex;
	lIndex = 0;
	PopupHistoryData *popupItem;
	for (i = 0; i < lstPopupHistory.Count(); i++)
	{
		item.iItem = lIndex;
		popupItem = lstPopupHistory.Get(i);
		if (MatchesFilter(filter, popupItem))
		{
			item.pszText = popupItem->titleT;
			ListView_InsertItem(hHistoryList, &item);
			ListView_SetItemText(hHistoryList, lIndex, 1, popupItem->messageT);
			myTime = localtime(&popupItem->timestamp);
			_tcsftime(buffer, 1024, _T("%c"), myTime);
			ListView_SetItemText(hHistoryList, lIndex++, 2, buffer);
		}
	}
}

void RefreshPopupHistory(HWND hWnd, int renderer)
{
	if (!hWnd) { return; }
	const int MAX_FILTER_SIZE = 1024;
	SIG_MATCHESFILTER MatchesFilter = (IsDlgButtonChecked(hWnd, IDC_HISTORY_FILTER_CASESENSITIVE)) ? MatchesFilterCS : MatchesFilterCI; //case sensitive compare or not ?
	
	SIG_ADDEVENTS AddEvents = (renderer == RENDER_DEFAULT) ? AddEventsDefault : AddEventsCustomControl;

	TCHAR filter[MAX_FILTER_SIZE];
	DeleteOldEvents(hWnd, renderer); //delete events
		
	GetWindowText(GetDlgItem(hWnd, IDC_HISTORY_FILTER), filter, SIZEOF(filter)); //get filter text
	
	AddEvents(hWnd, renderer, filter, MatchesFilter);
	
	if (renderer == RENDER_DEFAULT)
	{
		HWND hHistoryList = GetDlgItem(hWnd, IDC_LST_HISTORY);
		SortParams params = {0};
		params.hList = hHistoryList;
		params.column = lastColumn;
		
		ListView_SortItemsEx(hHistoryList, PopupsCompare, &params);
	}
}

void CopyPopupDataToClipboard(HWND hList, int selection)
{
	if (!selection)
	{
		return;
	}
	
	if (!GetOpenClipboardWindow())
	{
		if (OpenClipboard(hList))
		{
			TCHAR buffer[2048];
			buffer[0] = _T('\0');
			TCHAR *clipboard;
			int i;
			int found = 0;
			int count = ListView_GetItemCount(hList);
			int textType;

			textType = CF_UNICODETEXT;

			
			for (i = 0; i < count; i++)
			{
				if (ListView_GetItemState(hList, i, LVIS_SELECTED))
				{
					ListView_GetItemText(hList, i, selection - 100, buffer, SIZEOF(buffer));
					found = 1;
					break;
				}
			}
			if (found)
			{
				EmptyClipboard();
				int len = (int)_tcslen(buffer);
				
				HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, (len + 2) * sizeof(TCHAR));
				clipboard = (TCHAR *) GlobalLock(hData);
				_tcsncpy(clipboard, buffer, len);
				clipboard[len] = _T('\0');
				GlobalUnlock(hData);
				if (!SetClipboardData(textType, hData))
				{
					PUShowMessage("Could not set clipboard data", SM_WARNING);
				}
			}
			CloseClipboard();
		}
		else{
			PUShowMessage("Could not open clipboard", SM_WARNING);
		}
	}
	else{
		PUShowMessage("The clipboard is not available", SM_WARNING);
	}
}

//subclass proc for the list view
static LRESULT CALLBACK PopupsListSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CONTEXTMENU:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			int selection;		

			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, POPUPMENU_TITLE, TranslateT("Copy title to clipboard"));
			AppendMenu(hMenu, MF_STRING, POPUPMENU_MESSAGE, TranslateT("Copy message to clipboard"));
			AppendMenu(hMenu, MF_STRING, POPUPMENU_TIMESTAMP, TranslateT("Copy timestamp to clipboard"));
			selection = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWnd, NULL);
			DestroyMenu(hMenu);
			if (selection)
			{
				CopyPopupDataToClipboard(hWnd, selection);
			}

			break;
		}

	case WM_KEYUP:
		switch (wParam) {
		case 'C':
			if (GetKeyState(VK_CONTROL))
				CopyPopupDataToClipboard(hWnd, POPUPMENU_MESSAGE);
			break;

		case VK_ESCAPE:
			SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);
			break;
		}
		break;

	case WM_SYSKEYDOWN:
		if (wParam == 'X')
			SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);
		break;
	}

	return mir_callNextSubclass(hWnd, PopupsListSubclassProc, msg, wParam, lParam);
}

//load the columns
void LoadHistoryColumns(HWND hHistoryList)
{
	LVCOLUMN col;
	col.mask = LVCF_TEXT | LVCF_WIDTH;
	int i;
	
	for (i  = 0; i < cHistoryColumns; i++)
	{
		col.pszText = TranslateTS(szHistoryColumns[i]);
		col.cx = cxHistoryColumns[i];
		ListView_InsertColumn(hHistoryList, i, &col);
	}
}

//this is the history list window handler
INT_PTR CALLBACK DlgProcHistLst(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing;

	switch (msg) {
	case WM_INITDIALOG:
		bInitializing = 1;
		{
			int renderer = lstPopupHistory.GetRenderer();

			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)IcoLib_GetIcon(ICO_HISTORY,0));
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)IcoLib_GetIcon(ICO_HISTORY,1));

			LoadRenderer(hWnd, renderer);

			TranslateDialogDefault(hWnd);
			HWND hHistoryList = GetDlgItem(hWnd, IDC_LST_HISTORY);

			ListView_SetExtendedListViewStyleEx(hHistoryList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

			mir_subclassWindow(hHistoryList, PopupsListSubclassProc);

			LoadHistoryColumns(hHistoryList);

			RefreshPopupHistory(hWnd, renderer);
		}
		bInitializing = 0;
		return TRUE;

	case WM_DESTROY:
		UnloadRenderer(hWnd, lstPopupHistory.GetRenderer());
		hHistoryWindow = NULL;
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_WINDOWPOSCHANGING:
		{
			WINDOWPOS *wndPos = (WINDOWPOS *) lParam;

			if (wndPos->cx < MIN_HISTORY_WIDTH)
				wndPos->cx = MIN_HISTORY_WIDTH;
			if (wndPos->cy < MIN_HISTORY_HEIGHT)
				wndPos->cy = MIN_HISTORY_HEIGHT;

			RECT rParent;
			GetWindowRect(hWnd, &rParent);
			HDWP hdWnds = BeginDeferWindowPos(4);

			AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_CLOSE), &rParent, wndPos, ANCHOR_RIGHT | ANCHOR_BOTTOM);
			AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_HISTORY_FILTER), &rParent, wndPos, ANCHOR_LEFT | ANCHOR_BOTTOM);
			AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_HISTORY_FILTER_CASESENSITIVE), &rParent, wndPos, ANCHOR_LEFT | ANCHOR_RIGHT | ANCHOR_BOTTOM);
			AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_LST_HISTORY), &rParent, wndPos, ANCHOR_ALL);

			EndDeferWindowPos(hdWnds);
			MoveCustomControl(hWnd, lstPopupHistory.GetRenderer()); //move the custom control too, if any
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case IDC_HISTORY_FILTER_CASESENSITIVE:
			if (HIWORD(wParam) == BN_CLICKED)
				RefreshPopupHistory(hWnd, lstPopupHistory.GetRenderer());

		case IDC_HISTORY_FILTER:
			if (HIWORD(wParam) == EN_CHANGE)
				if (!bInitializing)
					RefreshPopupHistory(hWnd, lstPopupHistory.GetRenderer());
			break;
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case IDC_LST_HISTORY:
			switch (((LPNMHDR)lParam)->code) {
			case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW lv = (LPNMLISTVIEW) lParam;
					int column = lv->iSubItem;
					SortParams params = {0};
					params.hList = GetDlgItem(hWnd, IDC_LST_HISTORY);
					params.column = column;

					ListView_SortItemsEx(params.hList, PopupsCompare, (LPARAM) &params);
					lastColumn = (params.column == lastColumn) ? -1 : params.column;

					break;
				}
			}
		}
		break;
	}

	return 0;
}
