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

#include "dlgHandlers.h"
#include "math.h"
#include <commctrl.h>

#define GAP_SIZE 2
#define MIN_HISTORY_WIDTH 350
#define MIN_HISTORY_HEIGHT 100

struct WorkerThreadData{
	HWND hWnd;
	HistoryWindowData *data;
	IEVIEWEVENT ieEvent;
};

int LoadName(HWND hWnd);
int CalcIEViewPos(IEVIEWWINDOW *ieWnd, HWND hMainWindow);
int LoadIEView(HWND hWnd);
int MoveIEView(HWND hWnd);
int DestroyIEView(HWND hWnd);
int LoadEvents(HWND hWnd);
int LoadPage(HWND hWnd, HANDLE hFirstEvent, long index, long shiftCount, long readCount, int direction);
int LoadNext(HWND hWnd);
int LoadPrev(HWND hWnd);
int ScrollToBottom(HWND hWnd);

void RefreshButtonStates(HWND hWnd);

HANDLE GetNeededEvent(HANDLE hLastFirstEvent, int index, int direction);

int CalcIEViewPos(IEVIEWWINDOW *ieWnd, HWND hMainWindow)
{
	RECT rect;
	GetWindowRect(GetDlgItem(hMainWindow, IDC_IEVIEW_PLACEHOLDER), &rect);
	ScreenToClient(hMainWindow, &rect);

	ieWnd->x = rect.left + GAP_SIZE;
	ieWnd->y = rect.top + GAP_SIZE;
	ieWnd->cx = rect.right - rect.left - (2 * GAP_SIZE);
	ieWnd->cy = rect.bottom - rect.top - (2 * GAP_SIZE);
	return 0;
}

int LoadName(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
	HANDLE hEvent = (HANDLE) CallService(MS_DB_EVENT_FINDFIRST, (WPARAM) data->hContact, 0);
	DBEVENTINFO event = {0};
	event.cbSize = sizeof(event);
	CallService(MS_DB_EVENT_GET, (WPARAM) hEvent, (LPARAM) &event); //to get the protocol
	TCHAR *szOther = GetContactName(data->hContact, event.szModule);
	TCHAR buffer[1024];
	_sntprintf(buffer, 1024, _T("%s: IEHistory"), szOther);
	SetWindowText(hWnd, buffer);
	free(szOther);
	return 0;
}

int LoadIEView(HWND hWnd)
{
	IEVIEWWINDOW ieWnd = {0};
	ieWnd.cbSize = sizeof(ieWnd);
	ieWnd.iType = IEW_CREATE;
	ieWnd.dwMode = IEWM_HISTORY;
	ieWnd.dwFlags = 0;
	ieWnd.parent = hWnd;
	CalcIEViewPos(&ieWnd, hWnd);
	
	CallService(MS_IEVIEW_WINDOW, 0, (LPARAM) &ieWnd);
	HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
	data->hIEView = ieWnd.hwnd;
	SetWindowPos(GetDlgItem(hWnd, IDC_IEVIEW_PLACEHOLDER), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	//ShowWindow(GetDlgItem(hWnd, IDC_IEVIEW_PLACEHOLDER), SW_HIDE);
	return 0;
}

int MoveIeView(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
	if (data)
		{
			IEVIEWWINDOW ieWnd = {0};
			ieWnd.cbSize = sizeof(ieWnd);
			ieWnd.parent = hWnd;
			ieWnd.hwnd = data->hIEView;
			ieWnd.iType = IEW_SETPOS;
			CalcIEViewPos(&ieWnd, hWnd);
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM) &ieWnd);
		}
	return 0;
}

int DestroyIEView(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
	IEVIEWWINDOW ieWnd = {0};
	ieWnd.cbSize = sizeof(ieWnd);
	ieWnd.parent = hWnd;
	ieWnd.hwnd = data->hIEView;
	ieWnd.iType = IEW_DESTROY;
	CallService(MS_IEVIEW_WINDOW, 0, (LPARAM) &ieWnd);
	return 0;
}

void FillIEViewInfo(IEVIEWEVENTDATA *fillData, DBEVENTINFO dbInfo, PBYTE blob)
{
	switch (dbInfo.eventType)
		{
			case EVENTTYPE_MESSAGE:
				fillData->iType = IEED_EVENT_MESSAGE;
				break;
			case EVENTTYPE_STATUS:
				fillData->iType = IEED_EVENT_STATUSCHANGE;
				break;
			case EVENTTYPE_FILE:
				fillData->iType = IEED_EVENT_FILE;
				break;
			case EVENTTYPE_URL:
				fillData->iType = IEED_EVENT_URL;
				break;
		}
	fillData->pszNick = "<nick here>";
	fillData->bIsMe = (dbInfo.flags & DBEF_SENT);
	fillData->dwFlags = (dbInfo.flags & DBEF_SENT) ? IEEDF_SENT : 0;
	fillData->time = dbInfo.timestamp;
	unsigned int len = strlen((char *) blob) + 1;
	PBYTE pos;
	
	fillData->pszText = (char *) blob;
//	fillData.pszText2 = (char *) blob;
	if (len < dbInfo.cbBlob)
		{
			pos = blob + len;
			fillData->pszTextW = (wchar_t *) pos;
//			fillData->pszText2W = (wchar_t *) pos;
			fillData->dwFlags |= IEEDF_UNICODE_TEXT;
		}
}

DWORD WINAPI WorkerThread(LPVOID lpvData)
{
	Log("%s", "Inside worker thread ...");
	WorkerThreadData *data = (WorkerThreadData *) lpvData;
	EnableWindow(GetDlgItem(data->hWnd, IDC_CLOSE), FALSE);
	const int LOAD_COUNT = 10;
	int count = 0;
	int target = data->ieEvent.count;
	int cLoad = LOAD_COUNT;
	int i;
	IEVIEWEVENT ieEvent = {0};
	IEVIEWEVENTDATA ieData[LOAD_COUNT] = {0};
	PBYTE messages[LOAD_COUNT] = {0};
	HANDLE dbEvent = data->ieEvent.hDbEventFirst;
	for (i = 0; i < LOAD_COUNT; i++)
		{
			ieData[i].cbSize = sizeof(IEVIEWEVENTDATA); //set the cbsize here, no need to do it every time
			ieData[i].next = &ieData[i + 1]; //it's a vector, so v[i]'s next element is v[i + 1]
		}
	ieData[LOAD_COUNT - 1].next = NULL;
	ieEvent = data->ieEvent;
	ieEvent.iType = IEE_LOG_MEM_EVENTS;
	ieEvent.eventData = ieData;
	DBEVENTINFO dbInfo = {0};
	dbInfo.cbSize = sizeof(DBEVENTINFO);
	PBYTE buffer = NULL;
	int newSize, oldSize = 0;
	while (count < target)
		{
			cLoad = (count + LOAD_COUNT > target) ? target - count : LOAD_COUNT;
			ieEvent.count = -1;
			
			for (i = 0; i < cLoad; i++)
				{
					newSize = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM) dbEvent, 0);
					if (newSize > oldSize)
						{
							buffer = (PBYTE) realloc(buffer, newSize);
							dbInfo.pBlob = buffer;
							oldSize = newSize;
						}
					messages[i] = (PBYTE) realloc(messages[i], newSize);
					dbInfo.cbBlob = newSize;
					if (!CallService(MS_DB_EVENT_GET, (WPARAM) dbEvent, (LPARAM) &dbInfo))
						{
							memmove(messages[i], dbInfo.pBlob, newSize);
							FillIEViewInfo(&ieData[i], dbInfo, messages[i]);
						}
					//FillIEViewEventData(&ieData[i], dbEvent);
					dbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) dbEvent, 0);
				}
			ieData[cLoad - 1].next = NULL; //cLoad < LOAD_COUNT will only happen once, at the end
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM) &ieEvent);
			count += cLoad;
		}
	for (i = 0; i < LOAD_COUNT; i++)
		{
			free(messages[i]);
		}
	free(buffer);
	EnableWindow(GetDlgItem(data->hWnd, IDC_CLOSE), TRUE);
	free(data);
	//RefreshButtonStates(data->hWnd);
	Log("%s", "WorkerThread finished ... returning");
	return 0;
}

int DoLoadEvents(HWND hWnd, HistoryWindowData *data, IEVIEWEVENT ieEvent)
{
	ieEvent.iType = IEE_CLEAR_LOG;
	CallService(MS_IEVIEW_EVENT, 0, (LPARAM) &ieEvent);
	if (data->loadMethod == LOAD_IN_BACKGROUND)
		{
			WorkerThreadData *threadData = (WorkerThreadData *) malloc(sizeof(WorkerThreadData));
			threadData->data = data;
			threadData->hWnd = hWnd;
			threadData->ieEvent = ieEvent;
			WorkerThread(threadData);
			/*
			DWORD threadID;
			HANDLE thread = CreateThread(NULL, 0, WorkerThread, threadData, 0, &threadID);
			if (!thread)
				{
					MessageBox(hWnd, TranslateT("An error occured while trying to create the worker thread (%m)"), TranslateT("Error"), MB_OK | MB_ICONERROR);
				} */
		}
		else{
			ieEvent.iType = IEE_LOG_DB_EVENTS;
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM) &ieEvent);
			ScrollToBottom(hWnd);
			
			TCHAR buffer[256];
			_itot(data->index + 1, buffer, 10);
			SendDlgItemMessage(hWnd, IDC_STATUSBAR, SB_SETTEXT, 0 | SBT_POPOUT, (LPARAM) buffer);
			_itot(data->index + ieEvent.count, buffer, 10);
			SendDlgItemMessage(hWnd, IDC_STATUSBAR, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM) buffer);
			_itot(data->count, buffer, 10);
			SendDlgItemMessage(hWnd, IDC_STATUSBAR, SB_SETTEXT, 3 | SBT_POPOUT, (LPARAM) buffer);
			RefreshButtonStates(hWnd);
		}
	return 0;
}

int LoadEvents(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
	int count = CallService(MS_DB_EVENT_GETCOUNT, (WPARAM) data->hContact, 0);
	int bLastFirst = DBGetContactSettingByte(NULL, ModuleName, "ShowLastPageFirst", 0);
	int bRTL = DBGetContactSettingByte(NULL, ModuleName, "EnableRTL", 0);
	bRTL = DBGetContactSettingByte(data->hContact, "Tab_SRMsg", "RTL", bRTL);
	data->bEnableRTL = bRTL;
	data->count = count;
	if (data->itemsPerPage > count)
		{
			data->itemsPerPage = count;
		}
	IEVIEWEVENT ieEvent = {0};
	ieEvent.cbSize = sizeof(ieEvent);
	ieEvent.hwnd = data->hIEView;
	ieEvent.hContact = data->hContact;
	ieEvent.count = (data->itemsPerPage <= 0) ? count : data->itemsPerPage;
	
	HANDLE hFirstEvent = (HANDLE) CallService(MS_DB_EVENT_FINDFIRST, (WPARAM) data->hContact, 0);
	int index = 0;
	if ((data->itemsPerPage > 0) && (bLastFirst))
		{
			index = data->count - data->itemsPerPage;
			hFirstEvent = GetNeededEvent(hFirstEvent, index, DIRECTION_FORWARD);
		}
	data->index = index;
	data->hLastFirstEvent = hFirstEvent;
	ieEvent.hDbEventFirst = hFirstEvent;
	if (data->bEnableRTL)
		{
			ieEvent.dwFlags |= IEEF_RTL;
		}
	DoLoadEvents(hWnd, data, ieEvent);
	return 0;
}

int LoadPage(HWND hWnd, HANDLE hFirstEvent, long index, long shiftCount, long readCount, int direction)
{
	HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
	int count = shiftCount;
	int newIndex = index;
	IEVIEWEVENT ieEvent = {0};
	ieEvent.cbSize = sizeof(ieEvent);
	ieEvent.hwnd = data->hIEView;
	ieEvent.hContact = data->hContact;
	
	if (direction == DIRECTION_BACK)
		{
			newIndex -= shiftCount;
			if (newIndex < 0)
				{
					newIndex = 0;
					count = index;
				}
		}
		else{
			newIndex += shiftCount;
			if (newIndex + readCount > data->count)
				{
					count = data->count - newIndex;
					newIndex = data->count - readCount;
				}
		}
	data->index = newIndex;
	HANDLE hEvent = GetNeededEvent(hFirstEvent, count, direction);
	data->hLastFirstEvent = hEvent;
	ieEvent.hDbEventFirst = hEvent;
	ieEvent.count = readCount;
	if (data->bEnableRTL)
		{
			ieEvent.dwFlags |= IEEF_RTL;
		}
	DoLoadEvents(hWnd, data, ieEvent);
	return 0;	
}

int LoadPrev(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
	LoadPage(hWnd, data->hLastFirstEvent, data->index, data->itemsPerPage, data->itemsPerPage, DIRECTION_BACK);
	int finish = data->index <= 0;
	return finish;
}

int LoadNext(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
	LoadPage(hWnd, data->hLastFirstEvent, data->index, data->itemsPerPage, data->itemsPerPage, DIRECTION_FORWARD);
	int finish = data->index + data->itemsPerPage >= data->count;
	return finish;
}

int ScrollToBottom(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
	IEVIEWWINDOW ieWnd = {0};
	ieWnd.cbSize = sizeof(ieWnd);
	ieWnd.iType = IEW_SCROLLBOTTOM;
	ieWnd.hwnd = data->hIEView;
	ieWnd.parent = hWnd;
	CallService(MS_IEVIEW_WINDOW, 0, (LPARAM) &ieWnd);
	return 0;
}

void AddAnchorWindowToDeferList(HDWP &hdWnds, HWND window, RECT *rParent, WINDOWPOS *wndPos, int anchors)
{
	RECT rChild = AnchorCalcPos(window, rParent, wndPos, anchors);
	hdWnds = DeferWindowPos(hdWnds, window, HWND_NOTOPMOST, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, SWP_NOZORDER);
}

void RefreshButtonStates(HWND hWnd)
{
	HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
	int bPrev = data->index > 0;
	int bNext = data->index + data->itemsPerPage < data->count;
	EnableWindow(GetDlgItem(hWnd, IDC_PREV), bPrev);
	EnableWindow(GetDlgItem(hWnd, IDC_NEXT), bNext);
}



BOOL CALLBACK HistoryDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_INITDIALOG:
				{
					Log("Inside WM_INITDIALOG ...");
					TranslateDialogDefault(hWnd);
					SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) hIcon);
					//SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					int bRTL = DBGetContactSettingByte(NULL, ModuleName, "EnableRTL", 0);
					if (bRTL)
						{
							SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_RTLREADING);
						}
					//InitCommonControls();
					HWND hStatusBar = CreateWindow(STATUSCLASSNAME, //class
																				_T("-"), //title
																				WS_CHILD | WS_VISIBLE | SBARS_TOOLTIPS | SBARS_SIZEGRIP, //style
																				0, 0, //x, y
																				0, 0, //width, height
																				hWnd, //parent
																				(HMENU) IDC_STATUSBAR, //menu
																				hInstance, //instance
																				NULL); //lpParam
					int x;
					int widths[] = {x = 50, x += 50, x += 150, -1};
					int count = sizeof(widths) / sizeof(widths[0]);
					SendMessage(hStatusBar, SB_SETPARTS, count, (LPARAM) widths);
					//SendMessage(hStatusBar, SB_SETTIPTEXT, 1, (LPARAM) TranslateT("First event shown in page"));
					//SendMessage(hStatusBar, SB_SETTIPTEXT, 2, (LPARAM) TranslateT("Last event shown in page"));
					SendMessage(hStatusBar, SB_SETTEXT, 2 | SBT_POPOUT, (LPARAM) TranslateT("Out of a total of"));
					return TRUE;
					break;
				}
			case WM_SHOWWINDOW:
				{
					Log("Inside WM_SHOWWINDOW ...");
					LoadName(hWnd);
					LoadIEView(hWnd);
					LoadEvents(hWnd);
					
					HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
					bool bAll = (data->itemsPerPage <= 0) || (data->itemsPerPage >= data->count);
					int bLastFirst = DBGetContactSettingByte(NULL, ModuleName, "ShowLastPageFirst", 0);
					if (!bLastFirst)
						{
							EnableWindow(GetDlgItem(hWnd, IDC_PREV), FALSE);
							EnableWindow(GetDlgItem(hWnd, IDC_NEXT), !bAll);
						}
						else{
							EnableWindow(GetDlgItem(hWnd, IDC_PREV), !bAll);
							EnableWindow(GetDlgItem(hWnd, IDC_NEXT), FALSE);
						}
					//ShowWindow(GetDlgItem(hWnd, IDC_PAGE_NUMBER), !bAll);
					EnableWindow(GetDlgItem(hWnd, IDC_SEARCH), !bAll);
					
					break;
				}
			case WM_DESTROY:
				{
					HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
					DestroyIEView(hWnd);
					free(data);
					WindowList_Remove(hOpenWindowsList, hWnd);
					break;
				}
			case WM_CLOSE:
				{
					if (IsWindowEnabled(GetDlgItem(hWnd, IDC_CLOSE)))
						{
							DestroyWindow(hWnd);
						}
						else{
							MessageBox(hWnd, TranslateT("You can't close the window now, wait for all events to load."), TranslateT("Error"), MB_OK | MB_ICONERROR);
						}
					break;
				}
			case WM_WINDOWPOSCHANGING:
				{
					HDWP hdWnds = BeginDeferWindowPos(6);
					RECT rParent;
					HWND hStatusBar = GetDlgItem(hWnd, IDC_STATUSBAR);
					WINDOWPOS *wndPos = (WINDOWPOS *) lParam;
					GetWindowRect(hWnd, &rParent);

					//hdWnds = DeferWindowPos(hdWnds, hStatusBar, HWND_NOTOPMOST, wndPos->x, wndPos->y + wndPos->cy - statusHeight, statusWidth, statusHeight, SWP_NOZORDER);
					SendMessage(hStatusBar, WM_SIZE, 0, 0);
					if (wndPos->cx < MIN_HISTORY_WIDTH)
						{
							wndPos->cx = MIN_HISTORY_WIDTH;
						}
					if (wndPos->cy < MIN_HISTORY_HEIGHT)
						{
							wndPos->cy = MIN_HISTORY_HEIGHT;
						}
					//MoveWindow(hStatusBar, wndPos->x, wndPos->y + wndPos->cy - statusHeight - 2, statusWidth - 2, statusHeight, TRUE);
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_STATUSBAR), &rParent, wndPos, ANCHOR_BOTTOM);
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_CLOSE), &rParent, wndPos, ANCHOR_RIGHT | ANCHOR_BOTTOM);
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_IEVIEW_PLACEHOLDER), &rParent, wndPos, ANCHOR_ALL);
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_PREV), &rParent, wndPos, ANCHOR_LEFT | ANCHOR_BOTTOM);
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_NEXT), &rParent, wndPos, ANCHOR_LEFT | ANCHOR_BOTTOM);
					//AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_PAGE_NUMBER), &rParent, wndPos, ANCHOR_LEFT | ANCHOR_BOTTOM);
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_SEARCH), &rParent, wndPos, ANCHOR_RIGHT | ANCHOR_BOTTOM);
					
					EndDeferWindowPos(hdWnds);
					MoveIeView(hWnd); 
					break;
				}
			case WM_COMMAND:
				{
					switch (LOWORD(wParam))
						{
							case IDC_CLOSE:
								{
									SendMessage(hWnd, WM_CLOSE, 0, 0);
									break;
								}
							case IDC_PREV:
								{
									int finished = LoadPrev(hWnd);
									//EnableWindow(GetDlgItem(hWnd, IDC_PREV), !finished);
									//EnableWindow(GetDlgItem(hWnd, IDC_NEXT), TRUE);
									break;
								}
							case IDC_NEXT:
								{
									int finished = LoadNext(hWnd);
									//EnableWindow(GetDlgItem(hWnd, IDC_NEXT), !finished);
									//EnableWindow(GetDlgItem(hWnd, IDC_PREV), TRUE);
									break;
								}
							case IDC_SEARCH:
								{
									HWND hSearch = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_SEARCH), hWnd, SearchDlgProc);
									if (hSearch == NULL)
										{
											char buffer[1024];
											sprintf(buffer, "Error #%d", GetLastError());
											MessageBoxA(0, buffer, "Error", MB_OK);
										}
									HistoryWindowData *data = (HistoryWindowData *) GetWindowLong(hWnd, DWL_USER);
									SearchWindowData *searchData = (SearchWindowData *) malloc(sizeof(SearchWindowData));
									searchData->hContact = data->hContact;
									searchData->hHistoryWindow = hWnd;
									searchData->hLastFoundEvent = NULL;
									searchData->index = 0;
									SetWindowLong(hSearch, DWL_USER, (LONG) searchData);
									ShowWindow(hSearch, SW_SHOW);
									//sprintf(buffer, "Error #%d", GetLastError());
									//MessageBoxA(0, buffer, "Error", MB_OK);
									break;
								}
						}
					break;
				}
			default:
				{
					break;
				}
		}
	return 0;
}

#include "prsht.h" //PSN_APPLY

BOOL CALLBACK OptionsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_INITDIALOG:
				{
					TranslateDialogDefault(hWnd);
					int count = DBGetContactSettingDword(NULL, ModuleName, "EventsToLoad", 0);
					EnableWindow(GetDlgItem(hWnd, IDC_EVENTS_COUNT), count > 0);
					EnableWindow(GetDlgItem(hWnd, IDC_SHOW_LAST_FIRST), count > 0);
					
					CheckDlgButton(hWnd, IDC_LOAD_ALL, count <= 0);
					CheckDlgButton(hWnd, IDC_LOAD_NUMBER, count > 0);
					CheckDlgButton(hWnd, IDC_ENABLE_RTL, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "EnableRTL", 0));
					CheckDlgButton(hWnd, IDC_SHOW_LAST_FIRST, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "ShowLastPageFirst", 0));
					CheckDlgButton(hWnd, IDC_LOAD_BACKGROUND, (BOOL) DBGetContactSettingByte(NULL, ModuleName, "UseWorkerThread", 0));
					TCHAR buffer[1024];
					_itot(count, buffer, 10);
					SetWindowText(GetDlgItem(hWnd, IDC_EVENTS_COUNT), buffer);
					
					break;
				}
			case WM_COMMAND:
				{
					switch (LOWORD(wParam))
						{
							case IDC_LOAD_ALL:
								{
									EnableWindow(GetDlgItem(hWnd, IDC_EVENTS_COUNT), FALSE);
									EnableWindow(GetDlgItem(hWnd, IDC_SHOW_LAST_FIRST), FALSE);
									SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
									break;
								}
							case IDC_LOAD_NUMBER:
								{
									EnableWindow(GetDlgItem(hWnd, IDC_EVENTS_COUNT), TRUE);
									EnableWindow(GetDlgItem(hWnd, IDC_SHOW_LAST_FIRST), TRUE);
									SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
									break;
								}
							case IDC_ENABLE_RTL:
							case IDC_SHOW_LAST_FIRST:
							case IDC_EVENTS_COUNT:
							case IDC_LOAD_BACKGROUND:
								{
									SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
									break;
								}
						}
					break;
				}
			case WM_NOTIFY:
				{
					switch(((LPNMHDR)lParam)->idFrom)
						{
							case 0:
								{
									switch (((LPNMHDR)lParam)->code)
										{
											case PSN_APPLY:
												{
													long count;
													if (IsDlgButtonChecked(hWnd, IDC_LOAD_ALL))
														{
															count = 0;
														}
														else{
															TCHAR buffer[1024];
															GetWindowText(GetDlgItem(hWnd, IDC_EVENTS_COUNT), buffer, sizeof(buffer));
															count = _tstol(buffer);
															count = (count < 0) ? 0 : count;
														}
													DBWriteContactSettingByte(NULL, ModuleName, "ShowLastPageFirst", IsDlgButtonChecked(hWnd, IDC_SHOW_LAST_FIRST));
													DBWriteContactSettingByte(NULL, ModuleName, "EnableRTL", IsDlgButtonChecked(hWnd, IDC_ENABLE_RTL));
													DBWriteContactSettingByte(NULL, ModuleName, "UseWorkerThread", IsDlgButtonChecked(hWnd, IDC_LOAD_BACKGROUND));
													DBWriteContactSettingDword(NULL, ModuleName, "EventsToLoad", count);
													
													break;
												}
										}
									break;
								}
						}
					break;
				}
			default:
				{
					break;
				}
		}
	return 0;
}


#include "commctrl.h" //tab control

BOOL CALLBACK SearchDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_INITDIALOG:
				{
					if (MyEnableThemeDialogTexture)
					{
						MyEnableThemeDialogTexture((HWND) hWnd, ETDT_ENABLETAB);
					}
					TranslateDialogDefault(hWnd);
					TCITEM tabItem = {0};
					tabItem.pszText = TranslateT("Text search");
					tabItem.mask = TCIF_TEXT;
					//MessageBoxA(0, "Before TCM_INSERTITEM 1", "Error", MB_OK);
					SendMessage(GetDlgItem(hWnd, IDC_TABS), TCM_INSERTITEM, 0, (LPARAM) &tabItem);
					tabItem.pszText = TranslateT("Time search");
					//MessageBoxA(0, "Before TCM_INSERTITEM 2", "Error", MB_OK);
					SendMessage(GetDlgItem(hWnd, IDC_TABS), TCM_INSERTITEM, 1, (LPARAM) &tabItem);
					//SendMessage(GetDlgItem(hWnd, IDC_SEARCH_TIME), DTM_SETFORMAT, 0, (LPARAM) "HH:mm");
					//MessageBoxA(0, "Before CheckDlgButton", "Error", MB_OK);
					CheckDlgButton(hWnd, IDC_DIRECTION_DOWN, TRUE);
					return TRUE;
					break;
				}
			case WM_SHOWWINDOW:
				{
					SetFocus(GetDlgItem(hWnd, IDC_SEARCH_TEXT));
					break;
				}
			case WM_DESTROY:
				{
					SearchWindowData *data = (SearchWindowData *) GetWindowLong(hWnd, DWL_USER);
					free(data);
					//DestroyWindow(hWnd);
					break;
				}
			case WM_CLOSE:
				{
					DestroyWindow(hWnd);
					break;
				}
			case WM_NOTIFY:
				{
					switch (((LPNMHDR) lParam)->idFrom)
						{
							case IDC_SEARCH_DATE:
							case IDC_SEARCH_TIME:
								{
									SearchWindowData *data = (SearchWindowData *) GetWindowLong(hWnd, DWL_USER);
									data->hLastFoundEvent = NULL; //start from top if changes occur
									break;
								}
							case IDC_TABS:
								{
									switch (((LPNMHDR) lParam)->code)
										{
											case TCN_SELCHANGE:
												{
													int tab = SendMessage(GetDlgItem(hWnd, IDC_TABS), TCM_GETCURSEL, 0, 0);
													ShowWindow(GetDlgItem(hWnd, IDC_SEARCH_DATE), (tab == 1) ? SW_SHOW : SW_HIDE);
													ShowWindow(GetDlgItem(hWnd, IDC_SEARCH_TIME), (tab == 1) ? SW_SHOW : SW_HIDE);
													ShowWindow(GetDlgItem(hWnd, IDC_SEARCH_TEXT), (tab == 0) ? SW_SHOW : SW_HIDE);
													SendMessage(GetDlgItem(hWnd, IDC_SEARCH_TEXT_STATIC), WM_SETTEXT, 0, (LPARAM) ((tab == 0) ? _T("Text :") : _T("Time :")));
													break;
												}
										}
									break;
								}
						}
					break;
				}
			case WM_COMMAND:
				{
					switch (LOWORD(wParam))
						{
							case IDC_SEARCH_TEXT:
								{
									switch (HIWORD(wParam))
										{
											case EN_CHANGE:
												{
													SearchWindowData *data = (SearchWindowData *) GetWindowLong(hWnd, DWL_USER);
													data->hLastFoundEvent = NULL; //start from top if changes occur
													break;
												}
										}
									break;
								}
							case IDCANCEL:
								{
									SendMessage(hWnd, WM_CLOSE, 0, 0);
									break;
								}
							case IDC_FIND_NEXT:
								{
									SearchWindowData *data = (SearchWindowData *) GetWindowLong(hWnd, DWL_USER);
									const HistoryWindowData *histData = (HistoryWindowData *) GetWindowLong(data->hHistoryWindow, DWL_USER);
									int direction = IsDlgButtonChecked(hWnd, IDC_DIRECTION_UP) ? DIRECTION_BACK : DIRECTION_FORWARD;
									int tab = SendMessage(GetDlgItem(hWnd, IDC_TABS), TCM_GETCURSEL, 0, 0);
									int type = (tab == 0) ? SEARCH_TEXT : SEARCH_TIME;
									SearchResult searchResult;
									if (data->hLastFoundEvent == NULL)
										{
											data->index = (direction == DIRECTION_FORWARD) ? 0 : histData->count;
										}
										else{
											data->hLastFoundEvent = GetNeededEvent(data->hLastFoundEvent, 1, direction);
										}
											
									if (type == SEARCH_TEXT) //text search
										{
											TCHAR text[2048]; //TODO buffer overrun
											SendMessage(GetDlgItem(hWnd, IDC_SEARCH_TEXT), WM_GETTEXT, 2048, (LPARAM) text);
											searchResult = SearchHistory(data->hContact, data->hLastFoundEvent, text, direction, type);
										}
										else{//time search
											TimeSearchData tsData = {0};
											SYSTEMTIME date = {0}, time = {0};
											int res = SendMessage(GetDlgItem(hWnd, IDC_SEARCH_DATE), DTM_GETSYSTEMTIME, 0, (LPARAM) &date);
											tsData.flags = (res == GDT_VALID) ? TSDF_DATE_SET : 0;
											res = SendMessage(GetDlgItem(hWnd, IDC_SEARCH_TIME), DTM_GETSYSTEMTIME, 0, (LPARAM) &time);
											if (res == GDT_VALID)
												{
													tsData.flags |= TSDF_TIME_SET;
													date.wHour = time.wHour;
													date.wMinute = time.wMinute;
													date.wSecond = time.wSecond;
													date.wMilliseconds = time.wMilliseconds;
												}
											tsData.time = date;
											searchResult = SearchHistory(data->hContact, data->hLastFoundEvent, &tsData, direction, type);
										}
									if (searchResult.hEvent)
										{
											//char buffer[1024];
											//sprintf(buffer, "Found it: index = %ld hEvent = %p", searchResult.index, searchResult.hEvent);
											//MessageBox(0, buffer, "Yupppi", 0);
											data->index += (direction == DIRECTION_BACK) ? -searchResult.index : searchResult.index;
											LoadPage(data->hHistoryWindow, searchResult.hEvent, data->index, histData->itemsPerPage / 2, histData->itemsPerPage, DIRECTION_BACK);
										}
										else{
											MessageBox(0, TranslateT("Search finished. No more entries ..."), TranslateT("Information"), MB_OK | MB_ICONINFORMATION);
										}
									data->hLastFoundEvent = searchResult.hEvent;
									break;
								}
						}
					break;
				}
		}
	return 0;
}