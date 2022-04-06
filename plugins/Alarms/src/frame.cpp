#include "stdafx.h"

HWND hwnd_plugin = nullptr;
HWND hwnd_frame = nullptr;
HWND hwnd_list = nullptr;

int frame_id = -1;

FontID font_id;
ColourID framebk_colour_id;
HFONT hFont = nullptr;
COLORREF fontColour, framebk;
HBRUSH bk_brush = nullptr;

#define CLUIFrameTitleBarClassName				"CLUIFrameTitleBar"

AlarmList alarm_list;
mir_cs list_cs;

HGENMENU hMenuShowReminders = nullptr;

#define ID_FRAME_UPDATE_TIMER   1011
#define ID_FRAME_SHOWHIDE_TIMER 1012

#define WMU_FILL_LIST  (WM_USER + 10)
#define WMU_SIZE_LIST  (WM_USER + 11)
#define WMU_INITIALIZE (WM_USER + 12)

void FixMainMenu();

int height_client_to_frame(int client_height, LONG style, LONG ex_style)
{
	RECT tr;
	tr.top = tr.right = tr.left = 0;
	tr.bottom = client_height;
	if (AdjustWindowRectEx(&tr, style, FALSE, ex_style))
		return tr.bottom - tr.top;
	return 0;
}

LRESULT CALLBACK FrameContainerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
		return TRUE;

	case WM_SHOWWINDOW:
		if (wParam) {
			g_plugin.setByte("ReminderFrameVisible", 1);
			Utils_RestoreWindowPosition(hwnd, 0, MODULENAME, "reminders_window");
			PostMessage(hwnd, WM_SIZE, 0, 0);
		}
		else {
			g_plugin.setByte("ReminderFrameVisible", 0);
			Utils_SaveWindowPosition(hwnd, 0, MODULENAME, "reminders_window");
		}
		break;
	case WM_SIZE:
		{
			HWND child = (HWND)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			RECT r;
			GetClientRect(hwnd, &r);

			SetWindowPos(child, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
			InvalidateRect(child, nullptr, TRUE);
			InvalidateRect(hwnd, nullptr, TRUE);
		}
		break;

	case WM_CLOSE:
		Utils_SaveWindowPosition(hwnd, 0, MODULENAME, "reminders_window");
		ShowWindow(hwnd, SW_HIDE);
		FixMainMenu();
		return TRUE;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool FrameIsFloating()
{
	if (frame_id == -1)
		return true; // no frames, always floating

	return CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, frame_id), 0) != 0;
}

ALARM context_menu_alarm = { 0 };

LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MEASUREITEMSTRUCT *mis;
	DRAWITEMSTRUCT *dis;
	SIZE textSize;
	SIZE timeSize;

	switch (msg) {

	case WM_CREATE:
		hwnd_list = CreateWindow(L"LISTBOX", L"",
			(WS_VISIBLE | WS_CHILD | LBS_NOINTEGRALHEIGHT | LBS_STANDARD | LBS_NOTIFY | LBS_OWNERDRAWFIXED) & ~LBS_SORT
			& ~WS_BORDER, 0, 0, 0, 0, hwnd, nullptr, g_plugin.getInst(), nullptr);
		return FALSE;

	case WMU_INITIALIZE:
		PostMessage(hwnd, WMU_FILL_LIST, 0, 0);
		SetTimer(hwnd, ID_FRAME_UPDATE_TIMER, 5000, nullptr);
		SetTimer(hwnd, ID_FRAME_SHOWHIDE_TIMER, 200, nullptr);
		return TRUE;

	case WM_MEASUREITEM:
		mis = (MEASUREITEMSTRUCT *)lParam;
		mis->itemHeight = options.row_height;
		return TRUE;

	case WM_DRAWITEM:
		dis = (DRAWITEMSTRUCT *)lParam;
		if (dis->itemID != (uint32_t)-1) {
			ALARM alarm = { 0 };
			mir_cslock lck(list_cs);
			ALARM &list_alarm = alarm_list.at(dis->itemData);
			copy_alarm_data(&alarm, &list_alarm);

			RECT r;
			GetClientRect(hwnd, &r);

			int min = MinutesInFuture(alarm.time, alarm.occurrence, alarm.day_mask);

			FillRect(dis->hDC, &dis->rcItem, bk_brush);

			dis->rcItem.left += options.indent;

			SetBkMode(dis->hDC, TRANSPARENT);
			SetTextColor(dis->hDC, fontColour);

			HICON hIcon = (min <= 5 ? hIconList2 : hIconList1);
			DrawIconEx(dis->hDC, dis->rcItem.left, (dis->rcItem.top + dis->rcItem.bottom - 16) >> 1, hIcon, 0, 0, 0, nullptr, DI_NORMAL);

			GetTextExtentPoint32(dis->hDC, alarm.szTitle, (int)mir_wstrlen(alarm.szTitle), &textSize);

			wchar_t buff[100];
			if (min >= 60)
				mir_snwprintf(buff, TranslateT("%dh %dm"), min / 60, min % 60);
			else
				mir_snwprintf(buff, TranslateT("%dm"), min);

			GetTextExtentPoint32(dis->hDC, buff, (int)mir_wstrlen(buff), &timeSize);

			if (textSize.cx > (dis->rcItem.right - dis->rcItem.left) - (GetSystemMetrics(SM_CXSMICON) + 4) - timeSize.cx - 2 - 4) {
				// need elipsis
				wchar_t titlebuff[512];
				size_t len = mir_wstrlen(alarm.szTitle);
				if (len > 511) len = 511;
				while (len > 0 && textSize.cx > (dis->rcItem.right - dis->rcItem.left) - (GetSystemMetrics(SM_CXSMICON) + 4) - timeSize.cx - 2 - 4) {
					len--;
					wcsncpy(titlebuff, alarm.szTitle, len);
					titlebuff[len] = 0;
					mir_wstrcat(titlebuff, L"...");
					GetTextExtentPoint32(dis->hDC, titlebuff, (int)mir_wstrlen(titlebuff), &textSize);
				}
				TextOut(dis->hDC, dis->rcItem.left + 16 + 4, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, titlebuff, (int)mir_wstrlen(titlebuff));
				TextOut(dis->hDC, dis->rcItem.right - timeSize.cx - 2, (dis->rcItem.top + dis->rcItem.bottom - timeSize.cy) >> 1, buff, (int)mir_wstrlen(buff));
			}
			else {
				TextOut(dis->hDC, dis->rcItem.left + 16 + 4, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, alarm.szTitle, (int)mir_wstrlen(alarm.szTitle));
				TextOut(dis->hDC, dis->rcItem.right - timeSize.cx - 2, (dis->rcItem.top + dis->rcItem.bottom - timeSize.cy) >> 1, buff, (int)mir_wstrlen(buff));
			}

			SetBkMode(dis->hDC, OPAQUE);

			free_alarm_data(&alarm);
		}
		else FillRect(dis->hDC, &dis->rcItem, bk_brush);

		return TRUE;

	case WM_CTLCOLORLISTBOX:
		break;

	case WM_ERASEBKGND:
		{
			HDC hdc = (HDC)wParam;
			RECT r;
			GetClientRect(hwnd, &r);
			FillRect(hdc, &r, bk_brush);
		}
		return TRUE;

	case WM_PRINTCLIENT:
		return TRUE;

	case WM_PAINT:
		{
			RECT r;
			if (GetUpdateRect(hwnd, &r, FALSE)) {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)hdc, PRF_CLIENT | PRF_CHILDREN);
				EndPaint(hwnd, &ps);
			}
		}
		return TRUE;

	case WM_SHOWWINDOW:
		if ((BOOL)wParam) PostMessage(hwnd, WMU_SIZE_LIST, 0, 0);
		break;

	case WM_SIZE:
		if (IsWindowVisible(hwnd)) {
			SendMessage(hwnd, WMU_SIZE_LIST, 0, 0);

			if (frame_id != -1)
				CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, FU_TBREDRAW);

			InvalidateRect(hwnd, nullptr, TRUE);
		}
		break;

	case WMU_SIZE_LIST:
		{
			if (SendMessage(hwnd_list, LB_GETITEMHEIGHT, 0, 0) != options.row_height)
				SendMessage(hwnd_list, LB_SETITEMHEIGHT, 0, options.row_height);

			int itemheight = SendMessage(hwnd_list, LB_GETITEMHEIGHT, 0, 0),
				count = SendMessage(hwnd_list, LB_GETCOUNT, 0, 0);

			if (options.auto_size_vert && IsWindowVisible(hwnd)) {
				if (FrameIsFloating()) {
					int height = height_client_to_frame(itemheight * count, GetWindowLongPtr(GetParent(hwnd), GWL_STYLE), GetWindowLongPtr(GetParent(hwnd), GWL_EXSTYLE));
					HWND titleBarHwnd = FindWindowEx(GetParent(hwnd), nullptr, _A2W(CLUIFrameTitleBarClassName), nullptr);
					if (titleBarHwnd) {
						RECT tbr;
						GetWindowRect(titleBarHwnd, &tbr);
						height += (tbr.bottom - tbr.top);
					}
					RECT rp_window;
					GetWindowRect(GetParent(hwnd), &rp_window);
					SetWindowPos(GetParent(hwnd), nullptr, 0, 0, rp_window.right - rp_window.left, height, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
				}
				else if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME) && frame_id != -1) {
					CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, frame_id), count * itemheight);
					CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, FU_TBREDRAW | FU_FMREDRAW | FU_FMPOS);
				}
			}

			RECT r, r2;
			GetClientRect(hwnd, &r);
			GetClientRect(hwnd_list, &r2);
			int width, height, winheight;

			width = r.right - r.left;
			winheight = r.bottom - r.top;

			height = min(count * itemheight, winheight - (winheight % itemheight));
			if (r2.right - r2.left != width || (r.bottom - r.top > 0 && r2.bottom - r2.top != height)) {
				SetWindowPos(hwnd_list, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
				InvalidateRect(hwnd_list, nullptr, FALSE);
			}

			if (options.auto_showhide) {
				if (ServiceExists(MS_CLIST_FRAMES_SHFRAME) && frame_id != -1) {
					if (IsWindowVisible(hwnd) && count == 0) {
						CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);
					}
					else if (!IsWindowVisible(hwnd) && count > 0) {
						// we have reminders - show if not linked to clist or if clist is visible
						if ((!options.hide_with_clist && FrameIsFloating()) || IsWindowVisible(g_clistApi.hwndContactList)) {
							CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);
							CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, FU_FMREDRAW | FU_FMPOS);
						}
					}
				}
				else {
					if (IsWindowVisible(hwnd) && count == 0)
						SetReminderFrameVisible(false);
					else if (!IsWindowVisible(hwnd) && count > 0)
						// we have reminders - show if not linked to clist or if clist is visible
						if (!options.hide_with_clist || IsWindowVisible(g_clistApi.hwndContactList))
							SetReminderFrameVisible(true);
				}
			}
		}
		return TRUE;

	case WMU_FILL_LIST:
		{
			// calculate the period to display alarms for
			SYSTEMTIME t1, t2;
			GetLocalTime(&t1);
			TimeForMinutesInFuture(60 * options.reminder_period, &t2);

			int sel = SendMessage(hwnd_list, LB_GETCURSEL, 0, 0),
				top = SendMessage(hwnd_list, LB_GETTOPINDEX, 0, 0);

			SendMessage(hwnd_list, WM_SETREDRAW, FALSE, 0);

			mir_cslock lck(list_cs);
			SendMessage(hwnd_list, LB_RESETCONTENT, 0, 0);
			copy_list(alarm_list, t1, t2);
			alarm_list.sort();
			int index = 0;
			ALARM *i;
			for (alarm_list.reset(); i = alarm_list.current(); alarm_list.next(), index++) {
				if (i->flags & (ALF_HIDDEN | ALF_SUSPENDED | ALF_NOREMINDER))
					continue;
				SendMessage(hwnd_list, LB_ADDSTRING, 0, (LPARAM)index);
			}

			SendMessage(hwnd, WMU_SIZE_LIST, 0, 0);
			SendMessage(hwnd_list, WM_SETREDRAW, TRUE, 0);

			if (sel != LB_ERR && sel < index) SendMessage(hwnd_list, LB_SETCURSEL, (WPARAM)sel, 0);
			if (top != LB_ERR && top < index) SendMessage(hwnd_list, LB_SETTOPINDEX, (WPARAM)top, 0);

		}
		return TRUE;

	case WM_TIMER:
		if (wParam == ID_FRAME_UPDATE_TIMER)
			SendMessage(hwnd, WMU_FILL_LIST, 0, 0);
		else if (wParam == ID_FRAME_SHOWHIDE_TIMER && options.hide_with_clist) { // link show/hide with clist
			// hide if we're visible and clist isn't (possible only when floating if frames are present)
			if (!IsWindowVisible(g_clistApi.hwndContactList) && IsWindowVisible(hwnd)) {
				if (ServiceExists(MS_CLIST_FRAMES_SHFRAME))
					CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);
				else
					SetReminderFrameVisible(false);
			}
			// we're not visible but clist is - show depending on hide_with_clist and auto_showhide options
			if (!IsWindowVisible(hwnd) && IsWindowVisible(g_clistApi.hwndContactList)) {
				// if not auto show/hide, show (reminders or not) if we're not visible and the clist is
				// otherwise, show only if there are reminders
				int count = SendMessage(hwnd_list, LB_GETCOUNT, 0, 0);
				if (!options.auto_showhide || count > 0) {
					if (ServiceExists(MS_CLIST_FRAMES_SHFRAME))
						CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);
					else
						SetReminderFrameVisible(true);
				}
			}
		}
		return TRUE;

	case WM_CONTEXTMENU:
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hwnd_list, &pt);
		{
			mir_cslock lck(list_cs);
			uint32_t item = SendMessage(hwnd_list, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));

			HMENU menu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MENU1)), submenu = GetSubMenu(menu, 0);
			TranslateMenu(submenu);

			if (HIWORD(item) == 0) {
				int sel = LOWORD(item);
				if (sel >= 0) {
					// one-off alarms can't be suspended
					int index = SendMessage(hwnd_list, LB_GETITEMDATA, (WPARAM)sel, 0);
					ALARM &list_alarm = alarm_list.at(index);
					copy_alarm_data(&context_menu_alarm, &list_alarm);
					if (context_menu_alarm.occurrence == OC_ONCE)
						EnableMenuItem(submenu, ID_REMINDERFRAMECONTEXT_SUSPEND, MF_BYCOMMAND | MF_GRAYED);

				}
			}
			else {
				EnableMenuItem(submenu, ID_REMINDERFRAMECONTEXT_SUSPEND, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(submenu, ID_REMINDERFRAMECONTEXT_EDIT, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(submenu, ID_REMINDERFRAMECONTEXT_DELETE, MF_BYCOMMAND | MF_GRAYED);
			}

			//ClientToScreen(hwnd_list, &pt);
			GetCursorPos(&pt);

			BOOL ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr);
			DestroyMenu(menu);
			if (ret)
				PostMessage(hwnd, WM_COMMAND, ret, 0);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_REMINDERFRAMECONTEXT_OPTIONS:
			g_plugin.openOptions(L"Events", L"Alarms");
			break;

		case ID_REMINDERFRAMECONTEXT_SUSPEND:
			if (context_menu_alarm.occurrence != OC_ONCE) {
				suspend(context_menu_alarm.id);
				PostMessage(hwnd, WMU_FILL_LIST, 0, 0);
				if (hwndOptionsDialog) {
					// refresh options list
					PostMessage(hwndOptionsDialog, WMU_INITOPTLIST, 0, 0);
				}
			}
			break;

		case ID_REMINDERFRAMECONTEXT_EDIT:
			EditNonModal(context_menu_alarm);
			break;

		case ID_REMINDERFRAMECONTEXT_DELETE:
			remove(context_menu_alarm.id);
			PostMessage(hwnd, WMU_FILL_LIST, 0, 0);
			if (hwndOptionsDialog) // refresh options list
				PostMessage(hwndOptionsDialog, WMU_INITOPTLIST, 0, 0);
			break;

		case ID_REMINDERFRAMECONTEXT_NEWALARM:
			NewAlarmMenuFunc(0, 0);
			break;
		}
		return TRUE;

	case WM_DESTROY:
		KillTimer(hwnd, ID_FRAME_UPDATE_TIMER);
		KillTimer(hwnd, ID_FRAME_SHOWHIDE_TIMER);
		free_alarm_data(&context_menu_alarm);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int ReloadFont(WPARAM, LPARAM)
{
	DeleteObject(hFont);

	LOGFONTA log_font;
	fontColour = Font_Get(font_id, &log_font);
	hFont = CreateFontIndirectA(&log_font);
	SendMessage(hwnd_list, WM_SETFONT, (WPARAM)hFont, TRUE);

	DeleteObject(bk_brush);
	bk_brush = CreateSolidBrush(db_get_dw(0, "Alarm", "clFrameBack", GetSysColor(COLOR_3DFACE)));
	RefreshReminderFrame();
	return 0;
}

void FixMainMenu()
{
	if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		if (options.hide_with_clist || options.auto_showhide)
			Menu_EnableItem(hMenuShowReminders, false);
		else
			Menu_ModifyItem(hMenuShowReminders,
				ReminderFrameVisible() ? LPGENW("Hide reminders") : LPGENW("Show reminders"), INVALID_HANDLE_VALUE, 0);
	}
}

/////////////////////////
// only used when no multiwindow functionality is available

bool ReminderFrameVisible()
{
	return IsWindowVisible(hwnd_frame) ? true : false;
}

void SetReminderFrameVisible(bool visible)
{
	if (frame_id == -1 && hwnd_frame != nullptr)
		ShowWindow(hwnd_frame, visible ? SW_SHOW : SW_HIDE);
}

INT_PTR ShowHideMenuFunc(WPARAM, LPARAM)
{
	if (ReminderFrameVisible())
		SendMessage(hwnd_frame, WM_CLOSE, 0, 0);
	else
		ShowWindow(hwnd_frame, SW_SHOW);

	FixMainMenu();
	return 0;
}

//////////////////////////////

int CreateFrame()
{
	WNDCLASS wndclass = {};
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hInstance = g_plugin.getInst();
	wndclass.lpfnWndProc = FrameWindowProc;
	wndclass.lpszClassName = L"AlarmsFrame";
	RegisterClass(&wndclass);

	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		hwnd_plugin = CreateWindow(L"AlarmsFrame", TranslateT("Alarms"),
			WS_CHILD | WS_CLIPCHILDREN,
			0, 0, 10, 10, g_clistApi.hwndContactList, nullptr, g_plugin.getInst(), nullptr);

		CLISTFrame Frame = { sizeof(CLISTFrame) };
		Frame.szName.a = LPGEN("Alarms");
		Frame.hWnd = hwnd_plugin;
		Frame.align = alBottom;
		Frame.Flags = F_VISIBLE | F_SHOWTB | F_SHOWTBTIP;
		Frame.height = 30;
		Frame.hIcon = hIconMenuSet;
		frame_id = g_plugin.addFrame(&Frame);
	}
	else {
		wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndclass.hInstance = g_plugin.getInst();
		wndclass.lpfnWndProc = FrameContainerWindowProc;
		wndclass.lpszClassName = L"AlarmsFrameContainer";
		RegisterClass(&wndclass);

		hwnd_frame = CreateWindowEx(WS_EX_TOOLWINDOW, L"AlarmsFrameContainer", TranslateT("Alarms"),
			(WS_POPUPWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN) & ~WS_VISIBLE,
			0, 0, 200, 100, g_clistApi.hwndContactList, nullptr, g_plugin.getInst(), nullptr);

		hwnd_plugin = CreateWindow(L"AlarmsFrame", TranslateT("Alarms"),
			WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
			0, 0, 10, 10, hwnd_frame, nullptr, g_plugin.getInst(), nullptr);

		SetWindowLongPtr(hwnd_frame, GWLP_USERDATA, (LONG_PTR)hwnd_plugin);

		///////////////////////
		// create menu item
		CreateServiceFunction(MODULENAME "/ShowHideReminders", ShowHideMenuFunc);

		CMenuItem mi(&g_plugin);
		mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Alarms"), 0);
		Menu_ConfigureItem(mi.root, MCI_OPT_UID, "8A3C1906-4809-4EE8-A32A-858003A2AAA7");

		SET_UID(mi, 0x27556ea9, 0xfa19, 0x4c2e, 0xb0, 0xc9, 0x48, 0x2, 0x5c, 0x17, 0xba, 0x5);
		mi.hIcolibItem = hIconMenuShowHide;
		mi.name.a = LPGEN("Show reminders");
		mi.pszService = MODULENAME "/ShowHideReminders";
		mi.position = 500010000;
		hMenuShowReminders = Menu_AddMainMenuItem(&mi);
		/////////////////////

		if (!options.auto_showhide) {
			if (options.hide_with_clist) {
				if (IsWindowVisible(g_clistApi.hwndContactList)) {
					ShowWindow(hwnd_frame, SW_SHOW);
					RefreshReminderFrame();
				}
				else ShowWindow(hwnd_frame, SW_HIDE);
			}
			else {
				if (g_plugin.getByte("ReminderFrameVisible", 1) == 1) {
					ShowWindow(hwnd_frame, SW_SHOW);
					RefreshReminderFrame();
				}
				else ShowWindow(hwnd_frame, SW_HIDE);
			}
		}

		FixMainMenu();
	}

	SendMessage(hwnd_plugin, WMU_INITIALIZE, 0, 0);

	strncpy_s(font_id.group, LPGEN("Frames"), _TRUNCATE);
	strncpy_s(font_id.name, LPGEN("Alarm reminders"), _TRUNCATE);
	strncpy_s(font_id.dbSettingsGroup, MODULENAME, _TRUNCATE);
	strncpy_s(font_id.setting, "Font", _TRUNCATE);
	font_id.order = 0;
	g_plugin.addFont(&font_id);

	strncpy_s(framebk_colour_id.dbSettingsGroup, MODULENAME, _TRUNCATE);
	strncpy_s(framebk_colour_id.group, LPGEN("Frames"), _TRUNCATE);
	strncpy_s(framebk_colour_id.name, LPGEN("Alarm reminders"), _TRUNCATE);
	strncpy_s(framebk_colour_id.setting, "clFrameBack", _TRUNCATE);
	framebk_colour_id.defcolour = GetSysColor(COLOR_3DFACE);
	framebk_colour_id.order = 0;
	g_plugin.addColor(&framebk_colour_id);

	LOGFONTA log_font;
	fontColour = Font_Get(font_id, &log_font);
	hFont = CreateFontIndirectA(&log_font);
	SendMessage(hwnd_list, WM_SETFONT, (WPARAM)hFont, TRUE);

	HookEvent(ME_FONT_RELOAD, ReloadFont);

	// create the brush used for the background in the absence of clist_modern skinning features - match clist
	bk_brush = CreateSolidBrush(db_get_dw(0, "Alarm", "clFrameBack", GetSysColor(COLOR_3DFACE)));

	SendMessage(hwnd_list, WM_SETFONT, (WPARAM)hFont, TRUE);

	return 0;
}

void RefreshReminderFrame()
{
	SendMessage(hwnd_plugin, WMU_FILL_LIST, 0, 0);

	if (frame_id == -1)
		InvalidateRect(hwnd_frame, nullptr, TRUE);
	else
		InvalidateRect(hwnd_plugin, nullptr, TRUE);
}

void InitFrames()
{
	CreateFrame();
}

void DeinitFrames()
{
	if (ServiceExists(MS_CLIST_FRAMES_REMOVEFRAME)) {
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)frame_id, 0);
	}
	DestroyWindow(hwnd_plugin);
	if (hwnd_frame) DestroyWindow(hwnd_frame);

	DeleteObject(bk_brush);
}
