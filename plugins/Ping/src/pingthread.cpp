#include "stdafx.h"

int upCount, total = 0;

size_t list_size = 0;

HANDLE hWakeEvent = nullptr;

// thread protected variables
mir_cs thread_finished_cs, list_changed_cs, data_list_cs;
bool thread_finished = false, list_changed = false;
PINGLIST data_list;

HANDLE status_update_thread = nullptr;

HWND hpwnd = nullptr, list_hwnd, hwnd_clist = nullptr;
int frame_id = -1;

HBRUSH tbrush = nullptr;

FontIDW font_id;
ColourIDW bk_col_id;
HFONT hFont = nullptr;
COLORREF bk_col = RGB(255, 255, 255);

////////////////
#define TM_AUTOALPHA  1
static int transparentFocus = 1;
/////////////////

bool get_thread_finished()
{
	mir_cslock lck(thread_finished_cs);
	bool retval = thread_finished;
	return retval;
}

void set_thread_finished(bool f)
{
	mir_cslock lck(thread_finished_cs);
	thread_finished = f;
}

bool get_list_changed()
{
	mir_cslock lck(list_changed_cs);
	bool retval = list_changed;
	return retval;
}

void set_list_changed(bool f)
{
	mir_cslock lck(list_changed_cs);
	list_changed = f;
}

void SetProtoStatus(wchar_t *pszLabel, char *pszProto, int if_status, int new_status)
{
	if (mir_strcmp(pszProto, Translate("<all>")) == 0) {
		for (auto &pa : Accounts())
			SetProtoStatus(pszLabel, pa->szModuleName, if_status, new_status);
	}
	else {
		if (Proto_GetStatus(pszProto) == if_status) {
			if (options.logging) {
				wchar_t buf[1024];
				mir_snwprintf(buf, TranslateT("%s - setting status of protocol '%S' (%d)"), pszLabel, pszProto, new_status);
				CallService(MODULENAME "/Log", (WPARAM)buf, 0);
			}
			CallProtoService(pszProto, PS_SETSTATUS, new_status, 0);
		}
	}
}

void __cdecl sttCheckStatusThreadProc(void*)
{
	MThreadHandle threadLock(status_update_thread);

	clock_t start_t = clock(), end_t;
	while (!get_thread_finished()) {
		end_t = clock();

		int wait = (int)((options.ping_period - ((end_t - start_t) / (double)CLOCKS_PER_SEC)) * 1000);
		if (wait > 0)
			WaitForSingleObjectEx(hWakeEvent, wait, TRUE);

		if (get_thread_finished()) break;

		start_t = clock();

		bool timeout = false;
		bool reply = false;
		int count = 0;

		PINGADDRESS pa = { 0 };
		HistPair history_entry;
		size_t size;
		{
			mir_cslock lck(data_list_cs);
			set_list_changed(false);
			size = data_list.size();
		}

		size_t index = 0;
		for (; index < size; index++) {
			{
				mir_cslock lck(data_list_cs);
				size_t c = 0;
				for (pinglist_it i = data_list.begin(); i != data_list.end() && c <= index; ++i, c++) {
					if (c == index) {
						// copy just what we need - i.e. not history, not command
						pa.get_status = i->get_status;
						pa.item_id = i->item_id;
						pa.miss_count = i->miss_count;
						pa.port = i->port;
						mir_wstrncpy(pa.pszLabel, i->pszLabel, _countof(pa.pszLabel));
						mir_wstrncpy(pa.pszName, i->pszName, _countof(pa.pszName));
						mir_strncpy(pa.pszProto, i->pszProto, _countof(pa.pszProto));
						pa.set_status = i->set_status;
						pa.status = i->status;
						break;
					}
				}
			}

			if (get_thread_finished()) break;
			if (get_list_changed()) break;

			if (pa.status != PS_DISABLED) {
				if (!options.no_test_icon) {
					mir_cslock lck(data_list_cs);
					for (pinglist_it i = data_list.begin(); i != data_list.end(); ++i)
						if (i->item_id == pa.item_id)
							i->status = PS_TESTING;

					InvalidateRect(list_hwnd, nullptr, FALSE);
				}

				CallService(MODULENAME "/Ping", 0, (LPARAM)&pa);

				if (get_thread_finished()) break;
				if (get_list_changed()) break;

				{	mir_cslock lck(data_list_cs);
				
					for (pinglist_it i = data_list.begin(); i != data_list.end(); ++i) {
						if (i->item_id == pa.item_id) {
							i->responding = pa.responding;
							i->round_trip_time = pa.round_trip_time;
							history_entry.first = i->round_trip_time;
							history_entry.second = time(0);
							history_map[i->item_id].push_back(history_entry);
							// maintain history (-1 represents no response)
							while (history_map[i->item_id].size() >= MAX_HISTORY)
								//history_map[i->item_id].pop_front();
								history_map[i->item_id].remove(history_map[i->item_id].begin().val());

							if (pa.responding) {
								if (pa.miss_count > 0)
									pa.miss_count = -1;
								else
									pa.miss_count--;
								pa.status = PS_RESPONDING;
							}
							else {
								if (pa.miss_count < 0)
									pa.miss_count = 1;
								else
									pa.miss_count++;
								pa.status = PS_NOTRESPONDING;
							}

							i->miss_count = pa.miss_count;
							i->status = pa.status;

							break;
						}
					}
				}

				if (pa.responding) {
					count++;

					if (pa.miss_count == -1 - options.retries ||
						(((-pa.miss_count) % (options.retries + 1)) == 0 && !options.block_reps)) {
						reply = true;
						if (options.show_popup2) {
							ShowPopup(TranslateT("Ping Reply"), pa.pszLabel, 1);
						}
					}
					if (pa.miss_count == -1 - options.retries && options.logging) {
						wchar_t buf[512];
						mir_snwprintf(buf, TranslateT("%s - reply, %d"), pa.pszLabel, pa.round_trip_time);
						CallService(MODULENAME "/Log", (WPARAM)buf, 0);
					}
					SetProtoStatus(pa.pszLabel, pa.pszProto, pa.get_status, pa.set_status);
				}
				else {
					if (pa.miss_count == 1 + options.retries ||
						((pa.miss_count % (options.retries + 1)) == 0 && !options.block_reps)) {
						timeout = true;
						if (options.show_popup)
							ShowPopup(TranslateT("Ping Timeout"), pa.pszLabel, 0);
					}
					if (pa.miss_count == 1 + options.retries && options.logging) {
						wchar_t buf[512];
						mir_snwprintf(buf, TranslateT("%s - timeout"), pa.pszLabel);
						CallService(MODULENAME "/Log", (WPARAM)buf, 0);
					}
				}

				InvalidateRect(list_hwnd, nullptr, FALSE);
			}
		}

		if (timeout) Skin_PlaySound("PingTimeout");
		if (reply) Skin_PlaySound("PingReply");

		if (!get_list_changed()) {
			upCount = count;
			total = (int)index;
		}
		else total = 0;
	}
}

bool FrameIsFloating()
{
	if (frame_id == -1)
		return true; // no frames, always floating

	return (CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, frame_id), 0) != 0);
}

int FillList(WPARAM, LPARAM)
{

	if (options.logging)
		CallService(MODULENAME "/Log", (WPARAM)L"ping address list reload", 0);

	PINGLIST pl;
	CallService(MODULENAME "/GetPingList", 0, (LPARAM)&pl);

	SendMessage(list_hwnd, WM_SETREDRAW, FALSE, 0);
	{
		mir_cslock lck(data_list_cs);
		data_list = pl;
		SendMessage(list_hwnd, LB_RESETCONTENT, 0, 0);

		int index = 0;
		for (pinglist_it j = data_list.begin(); j != data_list.end(); ++j, index++) {
			SendMessage(list_hwnd, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)&(*j));
		}
		set_list_changed(true);

		list_size = data_list.size();
	}
	SendMessage(list_hwnd, WM_SETREDRAW, TRUE, 0);

	InvalidateRect(list_hwnd, nullptr, FALSE);

	SetEvent(hWakeEvent);

	if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME) && options.attach_to_clist)
		UpdateFrame();

	return 0;
}

INT_PTR PingPlugShowWindow(WPARAM, LPARAM)
{
	if (hpwnd) {
		if (frame_id != -1 && ServiceExists(MS_CLIST_FRAMES_SHFRAME))
			CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);
		else
			ShowWindow(hpwnd, IsWindowVisible(hpwnd) ? SW_HIDE : SW_SHOW);
	}
	return 0;
}

#define TIMER_ID 11042

void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	if (frame_id != -1 && ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		wchar_t TBcapt[255];
		if (total > 0)
			mir_snwprintf(TBcapt, L"Ping (%d/%d)", upCount, total);
		else
			mir_snwprintf(TBcapt, L"Ping");

		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_TBNAME | FO_UNICODETEXT, frame_id), (LPARAM)TBcapt);
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_TBTIPNAME | FO_UNICODETEXT, frame_id), (LPARAM)TBcapt);
		CallService(MS_CLIST_FRAMES_UPDATEFRAME, frame_id, FU_TBREDRAW);
	}
}

DWORD context_point;
bool context_point_valid = false;
LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	MEASUREITEMSTRUCT *mis;
	LPDRAWITEMSTRUCT dis;
	SIZE textSize;
	RECT r;
	LPARAM lp;
	int sel;

	switch (msg) {

	case WM_MEASUREITEM:
		mis = (MEASUREITEMSTRUCT *)lParam;
		mis->itemWidth = 100;
		mis->itemHeight = options.row_height;
		return TRUE;

	case WM_DRAWITEM:
		dis = (LPDRAWITEMSTRUCT)lParam;
		if (dis->hwndItem == list_hwnd) {
			HBRUSH ttbrush = nullptr;
			COLORREF tcol;
			if (dis->itemID != -1) {
				PINGADDRESS itemData;
				{
					mir_cslock lck(data_list_cs);
					itemData = *(PINGADDRESS *)dis->itemData;
				}
				SendMessage(list_hwnd, LB_SETITEMHEIGHT, 0, (LPARAM)options.row_height);

				LONG x, y;
				if (context_point_valid) {
					GetWindowRect(list_hwnd, &r);
					x = LOWORD(context_point) - r.left;
					y = HIWORD(context_point) - r.top;
				}
				else x = y = 0;

				GetClientRect(hwnd, &r);

				if ((dis->itemState & ODS_SELECTED && dis->itemState & ODS_FOCUS)
					|| (context_point_valid && (x >= dis->rcItem.left && x <= dis->rcItem.right) && (y >= dis->rcItem.top && y <= dis->rcItem.bottom))) {
					tcol = db_get_dw(0, "CLC", "SelBkColour", GetSysColor(COLOR_HIGHLIGHT));
					SetBkColor(dis->hDC, tcol);
					FillRect(dis->hDC, &dis->rcItem, (ttbrush = CreateSolidBrush(tcol)));

					tcol = db_get_dw(0, "CLC", "SelTextColour", GetSysColor(COLOR_HIGHLIGHTTEXT));
					SetTextColor(dis->hDC, tcol);
				}
				else {
					tcol = bk_col;
					SetBkColor(dis->hDC, tcol);
					FillRect(dis->hDC, &dis->rcItem, (ttbrush = CreateSolidBrush(tcol)));

					tcol = g_plugin.getDword("FontCol", GetSysColor(COLOR_WINDOWTEXT));
					SetTextColor(dis->hDC, tcol);
				}

				SetBkMode(dis->hDC, TRANSPARENT);
				HICON hIcon = (itemData.status != PS_DISABLED ? (itemData.status == PS_TESTING ? hIconTesting : (itemData.status == PS_RESPONDING ? hIconResponding : hIconNotResponding)) : hIconDisabled);
				dis->rcItem.left += options.indent;
				DrawIconEx(dis->hDC, dis->rcItem.left, dis->rcItem.top + ((options.row_height - 16) >> 1), hIcon, 0, 0, 0, nullptr, DI_NORMAL);

				GetTextExtentPoint32(dis->hDC, itemData.pszLabel, (int)mir_wstrlen(itemData.pszLabel), &textSize);
				TextOut(dis->hDC, dis->rcItem.left + 16 + 4, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, itemData.pszLabel, (int)mir_wstrlen(itemData.pszLabel));

				if (itemData.status != PS_DISABLED) {
					wchar_t buf[256];
					if (itemData.responding) {
						mir_snwprintf(buf, TranslateT("%d ms"), itemData.round_trip_time);
						GetTextExtentPoint32(dis->hDC, buf, (int)mir_wstrlen(buf), &textSize);
						TextOut(dis->hDC, dis->rcItem.right - textSize.cx - 2, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, buf, (int)mir_wstrlen(buf));
					}
					else if (itemData.miss_count > 0) {
						mir_snwprintf(buf, L"[%d]", itemData.miss_count);
						GetTextExtentPoint32(dis->hDC, buf, (int)mir_wstrlen(buf), &textSize);
						TextOut(dis->hDC, dis->rcItem.right - textSize.cx - 2, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, buf, (int)mir_wstrlen(buf));
					}
				}
				SetBkMode(dis->hDC, OPAQUE);
			}
			if (ttbrush) DeleteObject(ttbrush);
			return TRUE;
		}
		return TRUE;

	case WM_CTLCOLORLISTBOX:
		if (tbrush)
			DeleteObject(tbrush);
		return (INT_PTR)(tbrush = CreateSolidBrush(bk_col));

	case WM_ERASEBKGND:
		GetClientRect(hwnd, &r);
		if (!tbrush)
			tbrush = CreateSolidBrush(bk_col);
		FillRect((HDC)wParam, &r, tbrush);
		return TRUE;

	case WM_CONTEXTMENU:
		{
			context_point = lParam;
			context_point_valid = true;
			InvalidateRect(list_hwnd, nullptr, FALSE);
			HMENU menu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MENU1)),
				submenu = GetSubMenu(menu, 0);

			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			GetClientRect(list_hwnd, &r);
			ScreenToClient(list_hwnd, &pt);

			PINGADDRESS itemData;
			DWORD item = SendMessage(list_hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
			bool found = false;
			if (HIWORD(item) == 0) {
				int count = LOWORD(item);
				mir_cslock lck(data_list_cs);
				if (count >= 0 && count < (int)data_list.size()) {
					itemData = *(PINGADDRESS *)SendMessage(list_hwnd, LB_GETITEMDATA, count, 0);

					found = true;
					EnableMenuItem(submenu, ID_MENU_GRAPH, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(submenu, ID_MENU_EDIT, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(submenu, ID_MENU_TOGGLE, MF_BYCOMMAND | MF_ENABLED);
					if (itemData.status == PS_DISABLED)
						ModifyMenu(submenu, ID_MENU_TOGGLE, MF_BYCOMMAND | MF_STRING, ID_MENU_TOGGLE, TranslateT("Enable"));
					else
						ModifyMenu(submenu, ID_MENU_TOGGLE, MF_BYCOMMAND | MF_STRING, ID_MENU_TOGGLE, TranslateT("Disable"));
				}
			}

			if (!found) {
				EnableMenuItem(submenu, ID_MENU_GRAPH, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(submenu, ID_MENU_EDIT, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(submenu, ID_MENU_TOGGLE, MF_BYCOMMAND | MF_GRAYED);
			}

			TranslateMenu(submenu);

			GetCursorPos(&pt);
			BOOL ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr);
			DestroyMenu(menu);
			if (ret)
				SendMessage(hwnd, WM_COMMAND, ret, 0);

			context_point_valid = false;
			InvalidateRect(list_hwnd, nullptr, FALSE);
		}

		return TRUE;

	case WM_SYSCOLORCHANGE:
		SendMessage(list_hwnd, msg, wParam, lParam);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_CREATE:
		list_hwnd = CreateWindow(L"LISTBOX", L"",
			//(WS_VISIBLE | WS_CHILD | LBS_NOINTEGRALHEIGHT| LBS_STANDARD | WS_CLIPCHILDREN | LBS_OWNERDRAWVARIABLE | LBS_NOTIFY) 
			(WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_OWNERDRAWFIXED | LBS_NOTIFY)
			& ~WS_BORDER, 0, 0, 0, 0, hwnd, nullptr, g_plugin.getInst(), nullptr);

		if (db_get_b(0, "CList", "Transparent", 0)) {
			if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {

			}
			else {
				#ifdef WS_EX_LAYERED
				SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
				#endif
				#ifdef LWA_ALPHA
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)db_get_b(0, "CList", "Alpha", SETTING_ALPHA_DEFAULT), LWA_ALPHA);
				#endif
			}
		}

		// timer to update titlebar
		if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
			SetTimer(hwnd, TIMER_ID, 1000, TimerProc);

		PostMessage(hwnd, WM_SIZE, 0, 0);
		return TRUE;

	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE) {
			if ((HWND)wParam != hwnd)
				if (db_get_b(0, "CList", "Transparent", SETTING_TRANSPARENT_DEFAULT))
					if (transparentFocus)
						SetTimer(hwnd, TM_AUTOALPHA, 250, nullptr);
		}
		else {
			if (db_get_b(0, "CList", "Transparent", SETTING_TRANSPARENT_DEFAULT)) {
				KillTimer(hwnd, TM_AUTOALPHA);
				#ifdef LWA_ALPHA
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)db_get_b(0, "CList", "Alpha", SETTING_ALPHA_DEFAULT), LWA_ALPHA);
				#endif
				transparentFocus = 1;
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_SETCURSOR:
		if (db_get_b(0, "CList", "Transparent", SETTING_TRANSPARENT_DEFAULT)) {
			if (!transparentFocus && GetForegroundWindow() != hwnd) {
				#ifdef LWA_ALPHA
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)db_get_b(0, "CList", "Alpha", SETTING_ALPHA_DEFAULT), LWA_ALPHA);
				#endif
				transparentFocus = 1;
				SetTimer(hwnd, TM_AUTOALPHA, 250, nullptr);
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_TIMER:
		if ((int)wParam == TM_AUTOALPHA) {
			int inwnd;

			if (GetForegroundWindow() == hwnd) {
				KillTimer(hwnd, TM_AUTOALPHA);
				inwnd = 1;
			}
			else {
				POINT pt;
				HWND hwndPt;
				pt.x = (short)LOWORD(GetMessagePos());
				pt.y = (short)HIWORD(GetMessagePos());
				hwndPt = WindowFromPoint(pt);
				inwnd = (hwndPt == hwnd || GetParent(hwndPt) == hwnd);
			}
			if (inwnd != transparentFocus) { //change
				transparentFocus = inwnd;
				#ifdef LWA_ALPHA
				if (transparentFocus) SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)db_get_b(0, "CList", "Alpha", SETTING_ALPHA_DEFAULT), LWA_ALPHA);
				else SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)db_get_b(0, "CList", "AutoAlpha", SETTING_AUTOALPHA_DEFAULT), LWA_ALPHA);
				#endif
			}
			if (!transparentFocus) KillTimer(hwnd, TM_AUTOALPHA);
			return TRUE;
		}
		return FALSE;

	case WM_SHOWWINDOW:
		{
			static int noRecurse = 0;
			if (lParam) break;
			if (noRecurse) break;
			if (!db_get_b(0, "CLUI", "FadeInOut", 0))
				break;
			#ifdef WS_EX_LAYERED
			if (GetWindowLongPtr(hwnd, GWL_EXSTYLE)&WS_EX_LAYERED) {
				DWORD thisTick, startTick;
				int sourceAlpha, destAlpha;
				if (wParam) {
					sourceAlpha = 0;
					destAlpha = (BYTE)db_get_b(0, "CList", "Alpha", SETTING_AUTOALPHA_DEFAULT);
					#ifdef LWA_ALPHA
					SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_ALPHA);
					#endif
					noRecurse = 1;
					ShowWindow(hwnd, SW_SHOW);
					noRecurse = 0;
				}
				else {
					sourceAlpha = (BYTE)db_get_b(0, "CList", "Alpha", SETTING_AUTOALPHA_DEFAULT);
					destAlpha = 0;
				}
				for (startTick = GetTickCount();;) {
					thisTick = GetTickCount();
					if (thisTick >= startTick + 200) break;
					#ifdef LWA_ALPHA
					SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)(sourceAlpha + (destAlpha - sourceAlpha)*(int)(thisTick - startTick) / 200), LWA_ALPHA);
					#endif
				}
				#ifdef LWA_ALPHA
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)destAlpha, LWA_ALPHA);
				#endif
			}
			else AnimateWindow(hwnd, 200, AW_BLEND | (wParam ? 0 : AW_HIDE));
			#endif
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_MENU_GRAPH:
			if (context_point_valid) {
				WORD x = LOWORD(context_point), y = HIWORD(context_point);
				GetWindowRect(list_hwnd, &r);
				DWORD item = SendMessage(list_hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(x - r.left, y - r.top));
				if (HIWORD(item) == 0) {
					int count = LOWORD(item);
					bool found = false;
					PINGADDRESS itemData;
					{
						mir_cslock lck(data_list_cs);
						if (count >= 0 && count < (int)data_list.size()) {
							itemData = *(PINGADDRESS *)SendMessage(list_hwnd, LB_GETITEMDATA, count, 0);
							found = true;
						}
					}
					if (found)
						CallService(MODULENAME "/ShowGraph", (WPARAM)itemData.item_id, (LPARAM)itemData.pszLabel);
				}
			}
			return TRUE;

		case ID_MENU_TOGGLE:
			if (context_point_valid) {
				WORD x = LOWORD(context_point), y = HIWORD(context_point);
				GetWindowRect(list_hwnd, &r);
				DWORD item = SendMessage(list_hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(x - r.left, y - r.top));
				if (HIWORD(item) == 0) {
					int count = LOWORD(item);

					mir_cslock lck(data_list_cs);
					if (count >= 0 && count < (int)data_list.size()) {
						PINGADDRESS itemData = *(PINGADDRESS *)SendMessage(list_hwnd, LB_GETITEMDATA, count, 0);
						CallService(MODULENAME "/ToggleEnabled", (WPARAM)itemData.item_id, 0);
					}
				}
			}
			return TRUE;

		case ID_MENU_EDIT:
			if (context_point_valid) {
				WORD x = LOWORD(context_point), y = HIWORD(context_point);
				GetWindowRect(list_hwnd, &r);
				DWORD item = SendMessage(list_hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(x - r.left, y - r.top));
				PINGADDRESS *temp = nullptr;
				if (HIWORD(item) == 0) {
					int count = LOWORD(item);
					{
						mir_cslock lck(data_list_cs);
						if (count >= 0 && count < (int)data_list.size()) {
							temp = (PINGADDRESS *)SendMessage(list_hwnd, LB_GETITEMDATA, count, 0);
						}
					}
					if (temp) {
						PINGADDRESS itemData = *temp;
						if (Edit(hwnd, itemData)) {
							mir_cslock lck(data_list_cs);
							*temp = itemData;
							CallService(MODULENAME "/SetAndSavePingList", (WPARAM)&data_list, 0);
						}
					}
				}
			}
			return TRUE;

		case ID_MENU_DISABLEALLPINGS:
			CallService(MODULENAME "/DisableAll", 0, 0);
			return TRUE;

		case ID_MENU_ENABLEALLPINGS:
			CallService(MODULENAME "/EnableAll", 0, 0);
			return TRUE;

		case ID_MENU_OPTIONS:
			g_plugin.openOptions(L"Network", L"Ping", L"Settings");
			return TRUE;

		case ID_MENU_DESTINATIONS:
			g_plugin.openOptions(L"Network", L"Ping", L"Hosts");
			return TRUE;
		}

		if (HIWORD(wParam) == LBN_DBLCLK) {
			sel = SendMessage(list_hwnd, LB_GETCURSEL, 0, 0);
			if (sel != LB_ERR) {
				lp = SendMessage(list_hwnd, LB_GETITEMDATA, sel, 0);
				if (lp != LB_ERR) {
					mir_cslock lck(data_list_cs);

					PINGADDRESS *pItemData = (PINGADDRESS *)lp;
					if (pItemData) {
						DWORD item_id = pItemData->item_id;

						int wake = CallService(MODULENAME "/DblClick", (WPARAM)item_id, 0);
						InvalidateRect(list_hwnd, nullptr, FALSE);
						if (wake) SetEvent(hWakeEvent);

						if (options.logging) {
							wchar_t buf[1024];
							mir_snwprintf(buf, L"%s - %s", pItemData->pszLabel, (wake ? TranslateT("enabled") : TranslateT("double clicked")));
							CallService(MODULENAME "/Log", (WPARAM)buf, 0);
						}
					}
				}
			}
			return TRUE;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_MOVE:		// needed for docked frames in clist_mw (not needed in clist_modern)
		if (FrameIsFloating())
			break;

	case WM_SIZE:
		GetClientRect(hwnd, &rect);
		{
			int winheight = rect.bottom - rect.top,
				itemheight = SendMessage(list_hwnd, LB_GETITEMHEIGHT, 0, 0),
				count = SendMessage(list_hwnd, LB_GETCOUNT, 0, 0),
				#ifdef min
				height = min(winheight - winheight % itemheight, itemheight * count);
			#else
				height = std::min(winheight - winheight % itemheight, itemheight * count);
			#endif
			SetWindowPos(list_hwnd, nullptr, rect.left, rect.top, rect.right - rect.left, height, SWP_NOZORDER);
			InvalidateRect(list_hwnd, nullptr, FALSE);
		}
		InvalidateRect(hwnd, nullptr, TRUE);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_DESTROY:
		if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			Utils_SaveWindowPosition(hwnd, 0, MODULENAME, "main_window");
		}

		KillTimer(hwnd, TIMER_ID);
		if (tbrush) DeleteObject(tbrush);

		DestroyWindow(list_hwnd);

		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_CLOSE:
		if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			Utils_SaveWindowPosition(hwnd, 0, MODULENAME, "main_window");
			ShowWindow(hwnd, SW_HIDE);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);

	};

	return(TRUE);
};

int ReloadFont(WPARAM, LPARAM)
{
	if (hFont) DeleteObject(hFont);

	LOGFONT log_font;
	Font_GetW(font_id, &log_font);
	hFont = CreateFontIndirect(&log_font);
	SendMessage(list_hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);

	bk_col = Colour_GetW(bk_col_id);
	RefreshWindow(0, 0);

	return 0;
}

int RefreshWindow(WPARAM, LPARAM)
{
	InvalidateRect(list_hwnd, nullptr, TRUE);
	InvalidateRect(hpwnd, nullptr, TRUE);
	return 0;
}

void UpdateFrame()
{
	if (IsWindowVisible(hwnd_clist) != IsWindowVisible(hpwnd))
		ShowWindow(hpwnd, IsWindowVisible(hwnd_clist) ? SW_SHOW : SW_HIDE);

	if (!IsWindowVisible(hpwnd)) return;

	RECT r_clist;
	GetWindowRect(hwnd_clist, &r_clist);
	RECT r_frame;
	GetWindowRect(hpwnd, &r_frame);
	int height = (int)list_size * options.row_height;
	if (GetWindowLongPtr(hpwnd, GWL_STYLE) & WS_BORDER) {
		RECT r_frame_client;
		GetClientRect(hpwnd, &r_frame_client);
		height += (r_frame.bottom - r_frame.top) - (r_frame_client.bottom - r_frame_client.top);
	}

	SetWindowPos(hpwnd, nullptr, r_clist.left, r_clist.top - height, (r_clist.right - r_clist.left), height, SWP_NOZORDER | SWP_NOACTIVATE);
}

// Subclass procedure 
LRESULT APIENTRY ClistSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SIZE || uMsg == WM_MOVE)
		UpdateFrame();

	if (uMsg == WM_NCCALCSIZE) { // possible window style change
		if (GetWindowLongPtr(hwnd_clist, GWL_STYLE) != GetWindowLong(hpwnd, GWL_STYLE)
			|| GetWindowLongPtr(hwnd_clist, GWL_STYLE) != GetWindowLongPtr(hpwnd, GWL_STYLE)) {
			SetWindowLongPtr(hpwnd, GWL_STYLE, GetWindowLongPtr(hwnd_clist, GWL_STYLE));
			SetWindowLongPtr(hpwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd_clist, GWL_EXSTYLE));
			SetWindowPos(hpwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		}
	}

	if (uMsg == WM_SHOWWINDOW)
		ShowWindow(hpwnd, wParam);

	return mir_callNextSubclass(hwnd, ClistSubclassProc, uMsg, wParam, lParam);
}

void AttachToClist(bool attach)
{
	if (!hpwnd) return;

	if (attach) {
		SetWindowLongPtr(hpwnd, GWL_STYLE, GetWindowLongPtr(hwnd_clist, GWL_STYLE));
		SetWindowLongPtr(hpwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd_clist, GWL_EXSTYLE));
		SetWindowPos(hpwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

		// subclass clist to trap move/size
		mir_subclassWindow(hwnd_clist, ClistSubclassProc);
		UpdateFrame();
	}
	else {
		SetWindowLongPtr(hpwnd, GWL_STYLE, (WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE | WS_CLIPCHILDREN));
		SetWindowLongPtr(hpwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
		SetWindowPos(hpwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	}
}

void InitList()
{
	hwnd_clist = g_clistApi.hwndContactList;

	WNDCLASS wndclass;

	wndclass.style = 0;
	wndclass.lpfnWndProc = FrameWindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hIcon = hIconResponding;
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wndclass.lpszMenuName = nullptr;
	wndclass.lpszClassName = _A2W(MODULENAME) L"WindowClass";
	RegisterClass(&wndclass);

	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		hpwnd = CreateWindow(_A2W(MODULENAME) L"WindowClass", L"Ping", (WS_BORDER | WS_CHILD | WS_CLIPCHILDREN), 0, 0, 0, 0, hwnd_clist, nullptr, g_plugin.getInst(), nullptr);

		CLISTFrame frame = { 0 };
		frame.cbSize = sizeof(CLISTFrame);
		frame.szName.a = MODULENAME;
		frame.szTBname.a = LPGEN("Ping");
		frame.hWnd = hpwnd;
		frame.align = alBottom;
		frame.Flags = F_VISIBLE | F_SHOWTB | F_SHOWTBTIP;
		frame.height = 30;
		frame_id = g_plugin.addFrame(&frame);
	}
	else {
		hpwnd = CreateWindowEx(WS_EX_TOOLWINDOW, _A2W(MODULENAME) L"WindowClass", L"Ping",
			(WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN),
			0, 0, 400, 300, hwnd_clist, nullptr, g_plugin.getInst(), nullptr);

		Utils_RestoreWindowPosition(hpwnd, 0, MODULENAME, "main_window");

		CreateServiceFunction(MODULENAME "/ShowWindow", PingPlugShowWindow);

		CMenuItem mi(&g_plugin);
		mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Ping"), 1000200001);
		Menu_ConfigureItem(mi.root, MCI_OPT_UID, "7CFBF239-86B5-48B2-8D5B-39E09A7DB514");

		SET_UID(mi, 0x4adbd753, 0x27d6, 0x457a, 0xa6, 0x6, 0xdf, 0x4f, 0x2c, 0xd8, 0xb9, 0x3b);
		mi.flags = CMIF_UNICODE;
		mi.position = 3000320001;
		mi.name.w = LPGENW("Show/Hide &Ping Window");
		mi.pszService = MODULENAME "/ShowWindow";
		Menu_AddMainMenuItem(&mi);

		if (options.attach_to_clist) AttachToClist(true);
		else ShowWindow(hpwnd, SW_SHOW);
	}

	mir_wstrncpy(font_id.group, LPGENW("Ping"), _countof(font_id.group));
	mir_wstrncpy(font_id.name, LPGENW("List"), _countof(font_id.name));
	mir_strncpy(font_id.dbSettingsGroup, "PING", _countof(font_id.dbSettingsGroup));
	mir_strncpy(font_id.setting, "Font", _countof(font_id.setting));
	mir_wstrncpy(font_id.backgroundGroup, L"Ping", _countof(font_id.backgroundGroup));
	mir_wstrncpy(font_id.backgroundName, L"Background", _countof(font_id.backgroundName));
	font_id.order = 0;
	font_id.flags = FIDF_DEFAULTVALID;
	font_id.deffontsettings.charset = DEFAULT_CHARSET;
	font_id.deffontsettings.size = -14;
	font_id.deffontsettings.style = 0;
	font_id.deffontsettings.colour = RGB(255, 255, 255);
	mir_wstrncpy(font_id.deffontsettings.szFace, L"Tahoma", _countof(font_id.deffontsettings.szFace));

	g_plugin.addFont(&font_id);

	mir_wstrncpy(bk_col_id.group, L"Ping", _countof(bk_col_id.group));
	mir_wstrncpy(bk_col_id.name, L"Background", _countof(bk_col_id.name));
	mir_strncpy(bk_col_id.dbSettingsGroup, "PING", _countof(bk_col_id.dbSettingsGroup));
	mir_strncpy(bk_col_id.setting, "BgColor", _countof(bk_col_id.setting));
	bk_col_id.defcolour = RGB(0, 0, 0);
	g_plugin.addColor(&bk_col_id);

	HookEvent(ME_FONT_RELOAD, ReloadFont);

	ReloadFont(0, 0);

	mir_forkthread(sttCheckStatusThreadProc);
}

void DeinitList()
{
	DestroyWindow(hpwnd);

	SetEvent(hWakeEvent);
	if (status_update_thread) {
		WaitForSingleObject(status_update_thread, INFINITE);
		status_update_thread = nullptr;
	}

	if (hFont)
		DeleteObject(hFont);
}
