#include "common.h"

int upCount, total = 0;

size_t list_size = 0;

HANDLE mainThread;
HANDLE hWakeEvent = 0;

// thread protected variables
mir_cs thread_finished_cs, list_changed_cs, data_list_cs;
bool thread_finished = false, list_changed = false;
PINGLIST data_list;

HANDLE status_update_thread = 0;

HWND hpwnd = 0, list_hwnd, hwnd_clist = 0;
int frame_id = -1;

HBRUSH tbrush = 0;

FontIDT font_id;
ColourIDT bk_col_id;
HFONT hFont = 0;
COLORREF bk_col = RGB(255, 255, 255);

////////////////
#define TM_AUTOALPHA  1
static int transparentFocus = 1;
/////////////////

bool get_thread_finished() {
	mir_cslock lck(thread_finished_cs);
	bool retval = thread_finished;
	return retval;
}

void set_thread_finished(bool f) {
	mir_cslock lck(thread_finished_cs);
	thread_finished = f;
}

bool get_list_changed() {
	mir_cslock lck(list_changed_cs);
	bool retval = list_changed;
	return retval;
}

void set_list_changed(bool f) {
	mir_cslock lck(list_changed_cs);
	list_changed = f;
}

void SetProtoStatus(TCHAR *pszLabel, char *pszProto, int if_status, int new_status) {
	if (mir_strcmp(pszProto, Translate("<all>")) == 0) {
		int num_protocols;
		PROTOACCOUNT **pppDesc;

		ProtoEnumAccounts(&num_protocols, &pppDesc);
		for (int i = 0; i < num_protocols; i++) {
			SetProtoStatus(pszLabel, pppDesc[i]->szModuleName, if_status, new_status);
		}
	}
	else {
		if (ProtoServiceExists(pszProto, PS_GETSTATUS)) {
			if (ProtoCallService(pszProto, PS_GETSTATUS, 0, 0) == if_status) {
				if (options.logging) {
					TCHAR buf[1024];
					mir_sntprintf(buf, SIZEOF(buf), TranslateT("%s - setting status of protocol '%S' (%d)"), pszLabel, pszProto, new_status);
					CallService(PLUG "/Log", (WPARAM)buf, 0);
				}
				ProtoCallService(pszProto, PS_SETSTATUS, new_status, 0);
			}
		}
	}
}

void __cdecl sttCheckStatusThreadProc(void *vp)
{
	clock_t start_t = clock(), end_t;
	while (!get_thread_finished())
	{
		end_t = clock();

		int wait = (int)((options.ping_period - ((end_t - start_t) / (double)CLOCKS_PER_SEC)) * 1000);
		if (wait > 0)
			WaitForSingleObjectEx(hWakeEvent, wait, TRUE);

		if (get_thread_finished()) break;

		start_t = clock();

		bool timeout = false;
		bool reply = false;
		int count = 0;

		PINGADDRESS pa;
		HistPair history_entry;

		mir_cslock lck(data_list_cs);
		set_list_changed(false);
		size_t size = data_list.size();

		size_t index = 0;
		for (; index < size; index++)
		{
			mir_cslock lck(data_list_cs);
			size_t c = 0;
			for (pinglist_it i = data_list.begin(); i != data_list.end() && c <= index; ++i, c++)
			{
				if (c == index)
				{
					// copy just what we need - i.e. not history, not command
					pa.get_status = i->get_status;
					pa.item_id = i->item_id;
					pa.miss_count = i->miss_count;
					pa.port = i->port;
					mir_tstrncpy(pa.pszLabel, i->pszLabel, SIZEOF(pa.pszLabel));
					mir_tstrncpy(pa.pszName, i->pszName, SIZEOF(pa.pszName));
					mir_strncpy(pa.pszProto, i->pszProto, SIZEOF(pa.pszProto));
					pa.set_status = i->set_status;
					pa.status = i->status;
					break;
				}

			}

			if (get_thread_finished()) break;
			if (get_list_changed()) break;

			if (pa.status != PS_DISABLED) {
				if (!options.no_test_icon) {
					mir_cslock lck(data_list_cs);
					for (pinglist_it i = data_list.begin(); i != data_list.end(); ++i)
					{
						if (i->item_id == pa.item_id)
						{
							i->status = PS_TESTING;
						}
					}
					InvalidateRect(list_hwnd, 0, FALSE);
				}

				CallService(PLUG "/Ping", 0, (LPARAM)&pa);

				if (get_thread_finished()) break;
				if (get_list_changed()) break;

				mir_cslock lck(data_list_cs);
				for (pinglist_it i = data_list.begin(); i != data_list.end(); ++i)
				{
					if (i->item_id == pa.item_id)
					{
						i->responding = pa.responding;
						i->round_trip_time = pa.round_trip_time;
						history_entry.first = i->round_trip_time;
						history_entry.second = time(0);
						history_map[i->item_id].push_back(history_entry);
						// maintain history (-1 represents no response)
						while (history_map[i->item_id].size() >= MAX_HISTORY)
							//history_map[i->item_id].pop_front();
							history_map[i->item_id].remove(history_map[i->item_id].begin().val());

						if (pa.responding)
						{
							if (pa.miss_count > 0)
								pa.miss_count = -1;
							else
								pa.miss_count--;
							pa.status = PS_RESPONDING;
						}
						else
						{
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

				if (pa.responding) {
					count++;

					if (pa.miss_count == -1 - options.retries ||
						(((-pa.miss_count) % (options.retries + 1)) == 0 && !options.block_reps))
					{
						reply = true;
						if (options.show_popup2 && ServiceExists(MS_POPUP_SHOWMESSAGE)) {
							ShowPopup(TranslateT("Ping Reply"), pa.pszLabel, 1);
						}
					}
					if (pa.miss_count == -1 - options.retries && options.logging) {
						TCHAR buf[512];
						mir_sntprintf(buf, SIZEOF(buf), TranslateT("%s - reply, %d"), pa.pszLabel, pa.round_trip_time);
						CallService(PLUG "/Log", (WPARAM)buf, 0);
					}
					SetProtoStatus(pa.pszLabel, pa.pszProto, pa.get_status, pa.set_status);
				}
				else {

					if (pa.miss_count == 1 + options.retries ||
						((pa.miss_count % (options.retries + 1)) == 0 && !options.block_reps))
					{
						timeout = true;
						if (options.show_popup)
							ShowPopup(TranslateT("Ping Timeout"), pa.pszLabel, 0);
					}
					if (pa.miss_count == 1 + options.retries && options.logging) {
						TCHAR buf[512];
						mir_sntprintf(buf, SIZEOF(buf), TranslateT("%s - timeout"), pa.pszLabel);
						CallService(PLUG "/Log", (WPARAM)buf, 0);
					}
				}

				InvalidateRect(list_hwnd, 0, FALSE);
			}
		}

		if (timeout) SkinPlaySound("PingTimeout");
		if (reply) SkinPlaySound("PingReply");

		if (!get_list_changed()) {
			upCount = count;
			total = index;
		}
		else {
			total = 0;
		}
	}
}

void start_ping_thread()
{
	if (status_update_thread)
		CloseHandle(status_update_thread);
	status_update_thread = mir_forkthread(sttCheckStatusThreadProc, 0);
}

void stop_ping_thread() {
	set_thread_finished(true);
	SetEvent(hWakeEvent);
	//ICMP::get_instance()->stop();
	WaitForSingleObject(status_update_thread, 2000);
	TerminateThread(status_update_thread, 0);
	CloseHandle(status_update_thread);
	status_update_thread = 0;
}

bool FrameIsFloating() {
	if (frame_id == -1)
		return true; // no frames, always floating

	return (CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, frame_id), 0) != 0);
}

int FillList(WPARAM wParam, LPARAM lParam) {

	if (options.logging)
		CallService(PLUG "/Log", (WPARAM)_T("ping address list reload"), 0);

	PINGLIST pl;
	CallService(PLUG "/GetPingList", 0, (LPARAM)&pl);

	SendMessage(list_hwnd, WM_SETREDRAW, FALSE, 0);
	mir_cslock lck(data_list_cs);

	data_list = pl;
	SendMessage(list_hwnd, LB_RESETCONTENT, 0, 0);

	int index = 0;
	for (pinglist_it j = data_list.begin(); j != data_list.end(); ++j, index++)
	{
		SendMessage(list_hwnd, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)&(*j));
	}
	set_list_changed(true);

	list_size = data_list.size();

	SendMessage(list_hwnd, WM_SETREDRAW, TRUE, 0);

	InvalidateRect(list_hwnd, 0, FALSE);

	SetEvent(hWakeEvent);

	if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME) && options.attach_to_clist)
		UpdateFrame();

	return 0;
}

INT_PTR PingPlugShowWindow(WPARAM wParam, LPARAM lParam)
{
	if (hpwnd)
	{
		if (frame_id != -1 && ServiceExists(MS_CLIST_FRAMES_SHFRAME))
			CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);
		else
			ShowWindow(hpwnd, IsWindowVisible(hpwnd) ? SW_HIDE : SW_SHOW);
	}
	return 0;
}

#define TIMER_ID		11042
void CALLBACK TimerProc(
	HWND hwnd,         // handle to window
	UINT uMsg,         // WM_TIMER message
	UINT_PTR idEvent,  // timer identifier
	DWORD dwTime       // current system time
	)
{
	if (frame_id != -1 && ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
	{
		TCHAR TBcapt[255];
		if (total > 0)
			mir_sntprintf(TBcapt, SIZEOF(TBcapt), _T("Ping (%d/%d)"), upCount, total);
		else
			mir_sntprintf(TBcapt, SIZEOF(TBcapt), _T("Ping"));

		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_TBNAME | FO_TCHAR, frame_id), (LPARAM)TBcapt);
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_TBTIPNAME | FO_TCHAR, frame_id), (LPARAM)TBcapt);
		CallService(MS_CLIST_FRAMES_UPDATEFRAME, frame_id, FU_TBREDRAW);
	}
	else {
		//		if(options.attach_to_clist) {
		//			AttachToClist(true);
		//		}
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
	PINGADDRESS itemData;
	RECT r;
	LPARAM lp;
	int sel;

	switch (msg)
	{

	case WM_MEASUREITEM:
		mis = (MEASUREITEMSTRUCT *)lParam;
		mis->itemWidth = 100;
		mis->itemHeight = options.row_height;
		return TRUE;

	case WM_DRAWITEM:
		dis = (LPDRAWITEMSTRUCT)lParam;
		if (dis->hwndItem == list_hwnd) {
			HBRUSH ttbrush = 0;
			COLORREF tcol;
			if (dis->itemID != -1) {
				mir_cslock lck(data_list_cs);
				itemData = *(PINGADDRESS *)dis->itemData;

				SendMessage(list_hwnd, LB_SETITEMHEIGHT, 0, (LPARAM)options.row_height);
				//dis->rcItem.bottom = dis->rcItem.top + options.row_height;

				LONG x, y;
				if (context_point_valid) {
					RECT r;
					GetWindowRect(list_hwnd, &r);
					x = LOWORD(context_point) - r.left,
						y = HIWORD(context_point) - r.top;
				}

				GetClientRect(hwnd, &r);

				if ((dis->itemState & ODS_SELECTED && dis->itemState & ODS_FOCUS)
					|| (context_point_valid && (x >= dis->rcItem.left && x <= dis->rcItem.right) && (y >= dis->rcItem.top && y <= dis->rcItem.bottom)))
				{
					tcol = db_get_dw(NULL, "CLC", "SelBkColour", GetSysColor(COLOR_HIGHLIGHT));
					SetBkColor(dis->hDC, tcol);
					FillRect(dis->hDC, &dis->rcItem, (ttbrush = CreateSolidBrush(tcol)));

					tcol = db_get_dw(NULL, "CLC", "SelTextColour", GetSysColor(COLOR_HIGHLIGHTTEXT));
					SetTextColor(dis->hDC, tcol);
				}
				else {
					tcol = bk_col;
					SetBkColor(dis->hDC, tcol);
					FillRect(dis->hDC, &dis->rcItem, (ttbrush = CreateSolidBrush(tcol)));

					tcol = db_get_dw(NULL, PLUG, "FontCol", GetSysColor(COLOR_WINDOWTEXT));
					SetTextColor(dis->hDC, tcol);
				}

				SetBkMode(dis->hDC, TRANSPARENT);
				HICON hIcon = (itemData.status != PS_DISABLED ? (itemData.status == PS_TESTING ? hIconTesting : (itemData.status == PS_RESPONDING ? hIconResponding : hIconNotResponding)) : hIconDisabled);
				dis->rcItem.left += options.indent;
				//DrawIconEx(dis->hDC,dis->rcItem.left,(dis->rcItem.top + dis->rcItem.bottom - GetSystemMetrics(SM_CYSMICON))>>1,hIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
				//DrawIconEx(dis->hDC,dis->rcItem.left,(dis->rcItem.top + dis->rcItem.bottom - GetSystemMetrics(SM_CYSMICON))>>1,hIcon,0, 0, 0, NULL, DI_NORMAL);
				DrawIconEx(dis->hDC, dis->rcItem.left, dis->rcItem.top + ((options.row_height - 16) >> 1), hIcon, 0, 0, 0, NULL, DI_NORMAL);

				GetTextExtentPoint32(dis->hDC, itemData.pszLabel, (int)mir_tstrlen(itemData.pszLabel), &textSize);
				TextOut(dis->hDC, dis->rcItem.left + 16 + 4, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, itemData.pszLabel, (int)mir_tstrlen(itemData.pszLabel));

				if (itemData.status != PS_DISABLED) {
					TCHAR buf[256];
					if (itemData.responding) {
						mir_sntprintf(buf, SIZEOF(buf), TranslateT("%d ms"), itemData.round_trip_time);
						GetTextExtentPoint32(dis->hDC, buf, (int)mir_tstrlen(buf), &textSize);
						TextOut(dis->hDC, dis->rcItem.right - textSize.cx - 2, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, buf, (int)mir_tstrlen(buf));
					}
					else if (itemData.miss_count > 0) {
						mir_sntprintf(buf, SIZEOF(buf), _T("[%d]"), itemData.miss_count);
						GetTextExtentPoint32(dis->hDC, buf, (int)mir_tstrlen(buf), &textSize);
						TextOut(dis->hDC, dis->rcItem.right - textSize.cx - 2, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, buf, (int)mir_tstrlen(buf));
					}
				}
				SetBkMode(dis->hDC, OPAQUE);
			}
			if (ttbrush) DeleteObject(ttbrush);
			return TRUE;
		}
		//return DefWindowProc(hwnd, msg, wParam, lParam);
		return TRUE;

	case WM_CTLCOLORLISTBOX:
	{
		if (tbrush) DeleteObject(tbrush);

		return (BOOL)(tbrush = CreateSolidBrush(bk_col));
	}

	case WM_ERASEBKGND:
	{
		RECT r;
		GetClientRect(hwnd, &r);
		if (!tbrush) tbrush = CreateSolidBrush(bk_col);
		FillRect((HDC)wParam, &r, tbrush);
	}
	return TRUE;

	case WM_CONTEXTMENU:
	{
		context_point = lParam;
		context_point_valid = true;
		InvalidateRect(list_hwnd, 0, FALSE);
		HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1)),
			submenu = GetSubMenu(menu, 0);

		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		RECT r;
		GetClientRect(list_hwnd, &r);
		ScreenToClient(list_hwnd, &pt);

		DWORD item = SendMessage(list_hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
		bool found = false;
		if (HIWORD(item) == 0) {
			int count = LOWORD(item);
			mir_cslock lck(data_list_cs);
			if (count >= 0 && count < (int)data_list.size()) {
				itemData = *(PINGADDRESS *)SendMessage(list_hwnd, LB_GETITEMDATA, count, 0);
				found = true;
			}
		}

		if (found) {
			EnableMenuItem(submenu, ID_MENU_GRAPH, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(submenu, ID_MENU_EDIT, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(submenu, ID_MENU_TOGGLE, MF_BYCOMMAND | MF_ENABLED);
			if (itemData.status == PS_DISABLED) {
				ModifyMenu(submenu, ID_MENU_TOGGLE, MF_BYCOMMAND | MF_STRING, ID_MENU_TOGGLE, TranslateT("Enable"));
			}
			else {
				ModifyMenu(submenu, ID_MENU_TOGGLE, MF_BYCOMMAND | MF_STRING, ID_MENU_TOGGLE, TranslateT("Disable"));
			}
		}
		else {
			EnableMenuItem(submenu, ID_MENU_GRAPH, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(submenu, ID_MENU_EDIT, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(submenu, ID_MENU_TOGGLE, MF_BYCOMMAND | MF_GRAYED);
		}

		TranslateMenu(submenu);

		//ClientToScreen(list_hwnd, &pt);
		GetCursorPos(&pt);
		BOOL ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
		DestroyMenu(menu);
		if (ret) {
			SendMessage(hwnd, WM_COMMAND, ret, 0);
		}
		context_point_valid = false;
		InvalidateRect(list_hwnd, 0, FALSE);
	}

	return TRUE;

	case WM_SYSCOLORCHANGE:
		SendMessage(list_hwnd, msg, wParam, lParam);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_CREATE:
		list_hwnd = CreateWindow(_T("LISTBOX"), _T(""),
			//(WS_VISIBLE | WS_CHILD | LBS_NOINTEGRALHEIGHT| LBS_STANDARD | WS_CLIPCHILDREN | LBS_OWNERDRAWVARIABLE | LBS_NOTIFY) 
			(WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_OWNERDRAWFIXED | LBS_NOTIFY)
			& ~WS_BORDER, 0, 0, 0, 0, hwnd, NULL, hInst, 0);

		if (db_get_b(NULL, "CList", "Transparent", 0))
		{
			if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {

			}
			else {
#ifdef WS_EX_LAYERED
				SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
#endif
#ifdef LWA_ALPHA
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)db_get_b(NULL, "CList", "Alpha", SETTING_ALPHA_DEFAULT), LWA_ALPHA);
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
				if (db_get_b(NULL, "CList", "Transparent", SETTING_TRANSPARENT_DEFAULT))
					if (transparentFocus)
						SetTimer(hwnd, TM_AUTOALPHA, 250, NULL);
		}
		else {
			if (db_get_b(NULL, "CList", "Transparent", SETTING_TRANSPARENT_DEFAULT)) {
				KillTimer(hwnd, TM_AUTOALPHA);
#ifdef LWA_ALPHA
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)db_get_b(NULL, "CList", "Alpha", SETTING_ALPHA_DEFAULT), LWA_ALPHA);
#endif
				transparentFocus = 1;
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_SETCURSOR:
		if (db_get_b(NULL, "CList", "Transparent", SETTING_TRANSPARENT_DEFAULT)) {
			if (!transparentFocus && GetForegroundWindow() != hwnd) {
#ifdef LWA_ALPHA
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)db_get_b(NULL, "CList", "Alpha", SETTING_ALPHA_DEFAULT), LWA_ALPHA);
#endif
				transparentFocus = 1;
				SetTimer(hwnd, TM_AUTOALPHA, 250, NULL);
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_TIMER:
		if ((int)wParam == TM_AUTOALPHA)
		{
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
			if (inwnd != transparentFocus)
			{ //change
				transparentFocus = inwnd;
#ifdef LWA_ALPHA
				if (transparentFocus) SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)db_get_b(NULL, "CList", "Alpha", SETTING_ALPHA_DEFAULT), LWA_ALPHA);
				else SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)db_get_b(NULL, "CList", "AutoAlpha", SETTING_AUTOALPHA_DEFAULT), LWA_ALPHA);
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
		if (!db_get_b(NULL, "CLUI", "FadeInOut", 0))
			break;
#ifdef WS_EX_LAYERED
		if (GetWindowLongPtr(hwnd, GWL_EXSTYLE)&WS_EX_LAYERED) {
			DWORD thisTick, startTick;
			int sourceAlpha, destAlpha;
			if (wParam) {
				sourceAlpha = 0;
				destAlpha = (BYTE)db_get_b(NULL, "CList", "Alpha", SETTING_AUTOALPHA_DEFAULT);
#ifdef LWA_ALPHA
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_ALPHA);
#endif
				noRecurse = 1;
				ShowWindow(hwnd, SW_SHOW);
				noRecurse = 0;
			}
			else {
				sourceAlpha = (BYTE)db_get_b(NULL, "CList", "Alpha", SETTING_AUTOALPHA_DEFAULT);
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
		else {
			//			if(wParam) SetForegroundWindow(hwnd);
			AnimateWindow(hwnd, 200, AW_BLEND | (wParam ? 0 : AW_HIDE));
			//SetWindowPos(label,0,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
		}
#endif
		//int res = DefWindowProc(hwnd, msg, wParam, lParam);
		//return res;
		//break;
		//return FALSE; //break;
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	/*
	case WM_PAINT:
	{
	paintDC = BeginPaint(hwnd, &paintStruct); //
	//SelectObject(paintDC,TitleBarFont);
	//SetBkMode(paintDC,TRANSPARENT);

	//paintStruct.fErase=TRUE;
	//color=RGB(1,1,1);
	//brush=CreateSolidBrush(RGB(200,20,20));

	//GetClientRect(hwnd,&rect);
	//FillRect(paintDC,&rect,brush);
	//TextOut(paintDC,4,4,"cl1 Bottom window",sizeof("cl1 Bottom window")-1);
	//DeleteObject(brush);
	EndPaint(hwnd, &paintStruct); //

	};
	return TRUE;

	*/
	case WM_COMMAND:
		//CreateServiceFunction("PingPlug/DisableAll", PingPlugDisableAll);
		//CreateServiceFunction("PingPlug/EnableAll", PingPlugEnableAll);
		switch (LOWORD(wParam)) {
		case ID_MENU_GRAPH:
			if (context_point_valid) {
				WORD x = LOWORD(context_point),
					y = HIWORD(context_point);
				RECT r;
				GetWindowRect(list_hwnd, &r);
				DWORD item = SendMessage(list_hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(x - r.left, y - r.top));
				if (HIWORD(item) == 0) {
					int count = LOWORD(item);
					bool found = false;
					mir_cslock lck(data_list_cs);
					if (count >= 0 && count < (int)data_list.size()) {
						itemData = *(PINGADDRESS *)SendMessage(list_hwnd, LB_GETITEMDATA, count, 0);
						found = true;
					}
					if (found)
						CallService(PLUG "/ShowGraph", (WPARAM)itemData.item_id, (LPARAM)itemData.pszLabel);
				}
			}
			return TRUE;
		case ID_MENU_TOGGLE:
			if (context_point_valid) {
				WORD x = LOWORD(context_point),
					y = HIWORD(context_point);
				RECT r;
				GetWindowRect(list_hwnd, &r);
				DWORD item = SendMessage(list_hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(x - r.left, y - r.top));
				if (HIWORD(item) == 0) {
					int count = LOWORD(item);
					bool found = false;
					mir_cslock lck(data_list_cs);
					if (count >= 0 && count < (int)data_list.size()) {
						itemData = *(PINGADDRESS *)SendMessage(list_hwnd, LB_GETITEMDATA, count, 0);
						found = true;
					}
					if (found)
						CallService(PLUG "/ToggleEnabled", (WPARAM)itemData.item_id, 0);
				}
			}
			return TRUE;
		case ID_MENU_EDIT:
			if (context_point_valid) {
				WORD x = LOWORD(context_point),
					y = HIWORD(context_point);
				RECT r;
				GetWindowRect(list_hwnd, &r);
				DWORD item = SendMessage(list_hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(x - r.left, y - r.top));
				PINGADDRESS *temp = 0;
				if (HIWORD(item) == 0) {
					int count = LOWORD(item);
					mir_cslock lck(data_list_cs);
					if (count >= 0 && count < (int)data_list.size()) {
						temp = (PINGADDRESS *)SendMessage(list_hwnd, LB_GETITEMDATA, count, 0);
					}
					if (temp) {
						itemData = *temp;
						if (Edit(hwnd, itemData)) {
							mir_cslock lck(data_list_cs);
							*temp = itemData;
							CallService(PLUG "/SetAndSavePingList", (WPARAM)&data_list, 0);
						}
					}
				}
			}
			return TRUE;
		case ID_MENU_DISABLEALLPINGS:
			CallService(PLUG "/DisableAll", 0, 0);
			return TRUE;
		case ID_MENU_ENABLEALLPINGS:
			CallService(PLUG "/EnableAll", 0, 0);
			return TRUE;
		case ID_MENU_OPTIONS:
		{
			OPENOPTIONSDIALOG oop = { 0 };
			oop.cbSize = sizeof(oop);
			oop.pszGroup = "Network";
			oop.pszPage = "Ping";
			oop.pszTab = "Settings";
			Options_Open(&oop);
		}
		return TRUE;
		case ID_MENU_DESTINATIONS:
		{
			OPENOPTIONSDIALOG oop = { 0 };
			oop.cbSize = sizeof(oop);
			oop.pszGroup = "Network";
			oop.pszPage = "Ping";
			oop.pszTab = "Hosts";
			Options_Open(&oop);
		}
		return TRUE;
		}
		if (HIWORD(wParam) == LBN_DBLCLK) {
			sel = SendMessage(list_hwnd, LB_GETCURSEL, 0, 0);
			if (sel != LB_ERR) {
				lp = SendMessage(list_hwnd, LB_GETITEMDATA, sel, 0);
				if (lp != LB_ERR) {
					PINGADDRESS *pItemData = (PINGADDRESS *)lp;

					mir_cslock lck(data_list_cs);

					if (pItemData) {
						DWORD item_id = pItemData->item_id;

						int wake = CallService(PLUG "/DblClick", (WPARAM)item_id, 0);
						InvalidateRect(list_hwnd, 0, FALSE);
						if (wake) SetEvent(hWakeEvent);

						if (options.logging) {
							TCHAR buf[1024];
							mir_sntprintf(buf, SIZEOF(buf), _T("%s - %s"), pItemData->pszLabel, (wake ? TranslateT("enabled") : TranslateT("double clicked")));
							CallService(PLUG "/Log", (WPARAM)buf, 0);
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
	{
		//PostMessage(label, WM_SIZE, wParam, lParam);

		GetClientRect(hwnd, &rect);
		//SetWindowPos(list_hwnd, 0, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOZORDER);
		//InvalidateRect(list_hwnd, &rect, FALSE);
		int winheight = rect.bottom - rect.top,
			itemheight = SendMessage(list_hwnd, LB_GETITEMHEIGHT, 0, 0),
			count = SendMessage(list_hwnd, LB_GETCOUNT, 0, 0),
#ifdef min
			height = min(winheight - winheight % itemheight, itemheight * count);
#else
			height = std::min(winheight - winheight % itemheight, itemheight * count);
#endif
		SetWindowPos(list_hwnd, 0, rect.left, rect.top, rect.right - rect.left, height, SWP_NOZORDER);
		InvalidateRect(list_hwnd, 0, FALSE);
	}
	InvalidateRect(hwnd, 0, TRUE);
	return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_DESTROY:
		if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			Utils_SaveWindowPosition(hwnd, 0, PLUG, "main_window");
		}

		KillTimer(hwnd, TIMER_ID);
		if (tbrush) DeleteObject(tbrush);

		DestroyWindow(list_hwnd);

		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_CLOSE:
		if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			Utils_SaveWindowPosition(hwnd, 0, PLUG, "main_window");
			ShowWindow(hwnd, SW_HIDE);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
		/*
			case WM_POWERBROADCAST:

			if(options.logging) {
			std::ostringstream oss;
			switch(wParam) {
			case PBT_APMSUSPEND:
			CallService("PingPlug/Log", (WPARAM)"system suspend", 0);
			break;
			case PBT_APMRESUMESUSPEND:
			oss << "system resume";
			if(lParam == PBTF_APMRESUMEFROMFAILURE)
			oss << " [suspend failure!]";
			CallService("PingPlug/Log", (WPARAM)oss.str().c_str(), 0);
			break;
			case PBT_APMRESUMEAUTOMATIC:
			oss << "system resume (automatic)";
			if(lParam == PBTF_APMRESUMEFROMFAILURE)
			oss << " [suspend failure!]";
			CallService("PingPlug/Log", (WPARAM)oss.str().c_str(), 0);
			break;
			case PBT_APMRESUMECRITICAL:
			oss << "system resume (critical)";
			if(lParam == PBTF_APMRESUMEFROMFAILURE)
			oss << " [suspend failure!]";
			CallService("PingPlug/Log", (WPARAM)oss.str().c_str(), 0);
			break;
			}
			}
			break;
			*/
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);

	};

	return(TRUE);
};

int ReloadFont(WPARAM, LPARAM) {
	if (hFont) DeleteObject(hFont);

	LOGFONT log_font;
	CallService(MS_FONT_GETT, (WPARAM)&font_id, (LPARAM)&log_font);
	hFont = CreateFontIndirect(&log_font);
	SendMessage(list_hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);

	bk_col = CallService(MS_COLOUR_GETT, (WPARAM)&bk_col_id, 0);
	RefreshWindow(0, 0);

	return 0;
}

int RefreshWindow(WPARAM, LPARAM) {
	InvalidateRect(list_hwnd, 0, TRUE);
	InvalidateRect(hpwnd, 0, TRUE);
	return 0;
}

void UpdateFrame() {
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

	SetWindowPos(hpwnd, 0, r_clist.left, r_clist.top - height, (r_clist.right - r_clist.left), height, SWP_NOZORDER | SWP_NOACTIVATE);
}

// Subclass procedure 
LRESULT APIENTRY ClistSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SIZE || uMsg == WM_MOVE)
		UpdateFrame();

	if (uMsg == WM_NCCALCSIZE) { // possible window style change
		if (GetWindowLongPtr(hwnd_clist, GWL_STYLE) != GetWindowLong(hpwnd, GWL_STYLE)
			|| GetWindowLongPtr(hwnd_clist, GWL_STYLE) != GetWindowLongPtr(hpwnd, GWL_STYLE))
		{
			SetWindowLongPtr(hpwnd, GWL_STYLE, GetWindowLongPtr(hwnd_clist, GWL_STYLE));
			SetWindowLongPtr(hpwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd_clist, GWL_EXSTYLE));
			SetWindowPos(hpwnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
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
		SetWindowPos(hpwnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

		// subclass clist to trap move/size
		mir_subclassWindow(hwnd_clist, ClistSubclassProc);
		UpdateFrame();
	}
	else {
		SetWindowLongPtr(hpwnd, GWL_STYLE, (WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE | WS_CLIPCHILDREN));
		SetWindowLongPtr(hpwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
		SetWindowPos(hpwnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	}
}

void InitList()
{
	hwnd_clist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);

	WNDCLASS wndclass;

	wndclass.style = 0;
	wndclass.lpfnWndProc = FrameWindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInst;
	wndclass.hIcon = hIconResponding;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = _T(PLUG) _T("WindowClass");
	RegisterClass(&wndclass);

	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		hpwnd = CreateWindow(_T(PLUG) _T("WindowClass"), _T("Ping"), (WS_BORDER | WS_CHILD | WS_CLIPCHILDREN), 0, 0, 0, 0, hwnd_clist, NULL, hInst, NULL);

		CLISTFrame frame = { 0 };
		frame.name = PLUG;
		frame.cbSize = sizeof(CLISTFrame);
		frame.hWnd = hpwnd;
		frame.align = alBottom;
		frame.Flags = F_VISIBLE | F_SHOWTB | F_SHOWTBTIP;
		frame.height = 30;
		frame.TBname = Translate("Ping");

		frame_id = CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&frame, 0);
	}
	else {
		hpwnd = CreateWindowEx(WS_EX_TOOLWINDOW, _T(PLUG) _T("WindowClass"), _T("Ping"),
			(WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN),
			0, 0, 400, 300, hwnd_clist, NULL, hInst, NULL);

		Utils_RestoreWindowPosition(hpwnd, 0, PLUG, "main_window");

		CreateServiceFunction(PLUG "/ShowWindow", PingPlugShowWindow);

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIF_TCHAR;
		mi.popupPosition = 1000200001;
		mi.ptszPopupName = LPGENT("Ping");
		mi.position = 3000320001;
		mi.hIcon = 0;//LoadIcon( hInst, 0);
		mi.ptszName = LPGENT("Show/Hide &Ping Window");
		mi.pszService = PLUG "/ShowWindow";
		Menu_AddMainMenuItem(&mi);

		if (options.attach_to_clist) AttachToClist(true);
		else ShowWindow(hpwnd, SW_SHOW);
	}

	{
		font_id.cbSize = sizeof(FontIDT);
		mir_tstrncpy(font_id.group, LPGENT("Ping"), SIZEOF(font_id.group));
		mir_tstrncpy(font_id.name, LPGENT("List"), SIZEOF(font_id.name));
		mir_strncpy(font_id.dbSettingsGroup, "PING", SIZEOF(font_id.dbSettingsGroup));
		mir_strncpy(font_id.prefix, "Font", SIZEOF(font_id.prefix));
		mir_tstrncpy(font_id.backgroundGroup, _T("Ping"), SIZEOF(font_id.backgroundGroup));
		mir_tstrncpy(font_id.backgroundName, _T("Background"), SIZEOF(font_id.backgroundName));
		font_id.order = 0;
		font_id.flags = FIDF_DEFAULTVALID;
		font_id.deffontsettings.charset = DEFAULT_CHARSET;
		font_id.deffontsettings.size = -14;
		font_id.deffontsettings.style = 0;
		font_id.deffontsettings.colour = RGB(255, 255, 255);
		mir_tstrncpy(font_id.deffontsettings.szFace, _T("Tahoma"), SIZEOF(font_id.deffontsettings.szFace));

		FontRegisterT(&font_id);

		bk_col_id.cbSize = sizeof(ColourIDT);
		mir_tstrncpy(bk_col_id.group, _T("Ping"), SIZEOF(bk_col_id.group));
		mir_tstrncpy(bk_col_id.name, _T("Background"), SIZEOF(bk_col_id.name));
		mir_strncpy(bk_col_id.dbSettingsGroup, "PING", SIZEOF(bk_col_id.dbSettingsGroup));
		mir_strncpy(bk_col_id.setting, "BgColor", SIZEOF(bk_col_id.setting));
		bk_col_id.defcolour = RGB(0, 0, 0);
		ColourRegisterT(&bk_col_id);

		HookEvent(ME_FONT_RELOAD, ReloadFont);

		ReloadFont(0, 0);
	}

	start_ping_thread();
}

void DeinitList() {
	DestroyWindow(hpwnd);
	stop_ping_thread();
	if (hFont) DeleteObject(hFont);
}