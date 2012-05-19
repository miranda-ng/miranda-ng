#include "common.h"
#include "frame.h"

HWND hwnd_plugin = 0;
HWND hwnd_frame = 0;
HWND hwnd_list = 0;

int frame_id = -1;

FontID font_id;
ColourID framebk_colour_id;
HFONT hFont = 0;
COLORREF fontColour, framebk;
HBRUSH bk_brush = 0;

AlarmList alarm_list;
CRITICAL_SECTION list_cs;

HANDLE hMenuShowReminders = 0;

#define ID_FRAME_UPDATE_TIMER						1011
#define ID_FRAME_SHOWHIDE_TIMER						1012

#define WMU_FILL_LIST		(WM_USER + 10)
#define WMU_SIZE_LIST		(WM_USER + 11)
#define WMU_INITIALIZE		(WM_USER + 12)

void FixMainMenu();

int height_client_to_frame(int client_height, LONG style, LONG ex_style) {
	RECT tr;
	tr.top = tr.right = tr.left = 0;
	tr.bottom = client_height;
	if(AdjustWindowRectEx(&tr, style, FALSE, ex_style))
		return tr.bottom - tr.top;
	return 0;
}

LRESULT CALLBACK FrameContainerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_CREATE:
			return TRUE;
		case WM_SHOWWINDOW:
			if((BOOL)wParam) {
				DBWriteContactSettingByte(0, MODULE, "ReminderFrameVisible", 1);
				Utils_RestoreWindowPosition(hwnd, 0, MODULE, "reminders_window");
				PostMessage(hwnd, WM_SIZE, 0, 0);
			} else {
				DBWriteContactSettingByte(0, MODULE, "ReminderFrameVisible", 0);
				Utils_SaveWindowPosition(hwnd, 0, MODULE, "reminders_window");
			}
			break;
		case WM_SIZE:
			{
				HWND child = (HWND)GetWindowLong(hwnd, GWL_USERDATA);
				RECT r;
				GetClientRect(hwnd, &r);

				SetWindowPos(child, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
				InvalidateRect(child, 0, TRUE);
				InvalidateRect(hwnd, 0, TRUE);
			}
			break;

		case WM_CLOSE:
			Utils_SaveWindowPosition(hwnd, 0, MODULE, "reminders_window");
			ShowWindow(hwnd, SW_HIDE);
			FixMainMenu();
			return TRUE;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool FrameIsFloating() {
	if(frame_id == -1) 
		return true; // no frames, always floating
	
	return (CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, frame_id), 0) != 0);
}

ALARM context_menu_alarm = {0};

LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	MEASUREITEMSTRUCT *mis;
	DRAWITEMSTRUCT *dis;
	SIZE textSize;
	SIZE timeSize;

	switch(msg) {

		case WM_CREATE: 
			{
				hwnd_list = CreateWindow("LISTBOX", "",
					(WS_VISIBLE | WS_CHILD | LBS_NOINTEGRALHEIGHT | LBS_STANDARD | LBS_NOTIFY | LBS_OWNERDRAWFIXED) & ~LBS_SORT
					& ~WS_BORDER, 0, 0, 0, 0, hwnd, NULL, hInst,0);		
			}
			return FALSE;
		case WMU_INITIALIZE:
			PostMessage(hwnd, WMU_FILL_LIST, 0, 0);
			SetTimer(hwnd, ID_FRAME_UPDATE_TIMER, 5000, 0);
			SetTimer(hwnd, ID_FRAME_SHOWHIDE_TIMER, 200, 0);
			return TRUE;

		case WM_MEASUREITEM:
			mis = (MEASUREITEMSTRUCT *)lParam;
			mis->itemHeight = options.row_height;
			return TRUE;

		case WM_DRAWITEM:
			dis = (DRAWITEMSTRUCT *)lParam;
			if(dis->itemID != (DWORD)-1) {
				ALARM alarm = {0};
				EnterCriticalSection(&list_cs);
				ALARM &list_alarm = alarm_list.at(dis->itemData);
				copy_alarm_data(&alarm, &list_alarm);
				LeaveCriticalSection(&list_cs);

				RECT r;
				GetClientRect(hwnd, &r);

				int min = MinutesInFuture(alarm.time, alarm.occurrence);

				FillRect(dis->hDC, &dis->rcItem, bk_brush);

				dis->rcItem.left += options.indent;

				SetBkMode(dis->hDC, TRANSPARENT);
				SetTextColor(dis->hDC, fontColour);

				HICON hIcon = (min <= 5 ? hIconList2 : hIconList1);
				//DrawIconEx(dis->hDC,dis->rcItem.left,(dis->rcItem.top + dis->rcItem.bottom -GetSystemMetrics(SM_CYSMICON))>>1,hIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
				DrawIconEx(dis->hDC,dis->rcItem.left,(dis->rcItem.top + dis->rcItem.bottom - 16)>>1,hIcon,0, 0, 0, NULL, DI_NORMAL);

				GetTextExtentPoint32(dis->hDC,alarm.szTitle,lstrlen(alarm.szTitle),&textSize);

				char buff[100];
				if(min >= 60) {
					mir_snprintf(buff, 100, Translate("%dh %dm"), min / 60, min % 60);
				} else {
					mir_snprintf(buff, 100, Translate("%dm"), min);
				}

				GetTextExtentPoint32(dis->hDC,buff,lstrlen(buff),&timeSize);

				if(textSize.cx > (dis->rcItem.right - dis->rcItem.left) - (GetSystemMetrics(SM_CXSMICON) + 4) - timeSize.cx - 2 - 4) {
					// need elipsis
					char titlebuff[512];
					int len = lstrlen(alarm.szTitle);
					if(len > 511) len = 511;
					while(len > 0 && textSize.cx > (dis->rcItem.right - dis->rcItem.left) - (GetSystemMetrics(SM_CXSMICON) + 4) - timeSize.cx - 2 - 4) {
						len--;
						strncpy(titlebuff, alarm.szTitle, len);
						titlebuff[len] = 0;
						strcat(titlebuff, "...");
						GetTextExtentPoint32(dis->hDC,titlebuff,lstrlen(titlebuff),&textSize);
					}
					TextOut(dis->hDC,dis->rcItem.left + 16 + 4,(dis->rcItem.top + dis->rcItem.bottom - textSize.cy)>>1,titlebuff,lstrlen(titlebuff));
					TextOut(dis->hDC,dis->rcItem.right - timeSize.cx - 2,(dis->rcItem.top + dis->rcItem.bottom - timeSize.cy)>>1, buff,lstrlen(buff));
				} else {				
					TextOut(dis->hDC,dis->rcItem.left + 16 + 4,(dis->rcItem.top + dis->rcItem.bottom - textSize.cy)>>1,alarm.szTitle,lstrlen(alarm.szTitle));
					TextOut(dis->hDC,dis->rcItem.right - timeSize.cx - 2,(dis->rcItem.top + dis->rcItem.bottom - timeSize.cy)>>1, buff,lstrlen(buff));
				}

				SetBkMode(dis->hDC, OPAQUE);

				free_alarm_data(&alarm);
			} else {				
				FillRect(dis->hDC, &dis->rcItem, bk_brush);
			}
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
			{
				/*
				HDC hdc = (HDC)wParam;
				RECT r;
				GetClientRect(hwnd, &r);
				*/
			}
			return TRUE;

		case WM_PAINT:
			{
				RECT r;
				if(GetUpdateRect(hwnd, &r, FALSE)) {
					PAINTSTRUCT ps;
					HDC hdc = BeginPaint(hwnd, &ps);
					SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)hdc, (LPARAM)(PRF_CLIENT | PRF_CHILDREN));
					EndPaint(hwnd, &ps);
				}
			}
			return TRUE;

		case WM_SHOWWINDOW:
			if((BOOL)wParam) PostMessage(hwnd, WMU_SIZE_LIST, 0, 0);
			break;

		case WM_SIZE:
			if(IsWindowVisible(hwnd)) {
				SendMessage(hwnd, WMU_SIZE_LIST, 0, 0);
			
				if(frame_id != -1) {
					//CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)(FU_TBREDRAW | FU_FMREDRAW));
					CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)FU_TBREDRAW);
				}
				InvalidateRect(hwnd, 0, TRUE);
			}
			break;

		case WMU_SIZE_LIST:
			{
				if(SendMessage(hwnd_list, LB_GETITEMHEIGHT, 0, 0) != options.row_height)
					SendMessage(hwnd_list, LB_SETITEMHEIGHT, 0, options.row_height);

				int itemheight = SendMessage(hwnd_list, LB_GETITEMHEIGHT, 0, 0),
					count = SendMessage(hwnd_list, LB_GETCOUNT, 0, 0);
				
				if(options.auto_size_vert && IsWindowVisible(hwnd)) {
					if(FrameIsFloating()) {
						
#define CLUIFrameTitleBarClassName				"CLUIFrameTitleBar"
						
						int height = height_client_to_frame(itemheight * count, GetWindowLong(GetParent(hwnd), GWL_STYLE), GetWindowLong(GetParent(hwnd), GWL_EXSTYLE));
						HWND titleBarHwnd = FindWindowEx(GetParent(hwnd), 0, CLUIFrameTitleBarClassName, 0);
						if(titleBarHwnd) {
							RECT tbr;
							GetWindowRect(titleBarHwnd, &tbr);
							height += (tbr.bottom - tbr.top);
						}
						RECT rp_window;
						GetWindowRect(GetParent(hwnd), &rp_window);
						SetWindowPos(GetParent(hwnd), 0, 0, 0, rp_window.right - rp_window.left, height, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

						/*
						RECT rp_client, rp_window, r_window;
						GetClientRect(GetParent(hwnd), &rp_client);
						GetWindowRect(GetParent(hwnd), &rp_window);
						GetWindowRect(hwnd, &r_window);
						int diff = (rp_window.bottom - rp_window.top) - (rp_client.bottom - rp_client.top);
						if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
							diff += rp_window.top - r_window.top;
						SetWindowPos(GetParent(hwnd), 0, 0, 0, rp_window.right - rp_window.left, count * itemheight + diff, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
						*/
					} else if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME) && frame_id != -1) {
						int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
						CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, frame_id), (LPARAM)(count * itemheight));
						CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)(FU_TBREDRAW | FU_FMREDRAW | FU_FMPOS));
					}
				}

				RECT r, r2;
				GetClientRect(hwnd, &r);
				GetClientRect(hwnd_list, &r2);
				int width, height, winheight;

				width = r.right - r.left;
				winheight = r.bottom - r.top;

				height = min(count * itemheight, winheight - (winheight % itemheight));
				if(r2.right - r2.left != width || (r.bottom - r.top > 0 && r2.bottom - r2.top != height)) {
					SetWindowPos(hwnd_list, 0, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
					InvalidateRect(hwnd_list, 0, FALSE);
				}
				
				if(options.auto_showhide) {
					if(ServiceExists(MS_CLIST_FRAMES_SHFRAME) && frame_id != -1) {
						if(IsWindowVisible(hwnd) && count == 0) {
							CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);
						} else if(!IsWindowVisible(hwnd) && count > 0) {
							// we have reminders - show if not linked to clist or if clist is visible
							if((!options.hide_with_clist && FrameIsFloating()) || IsWindowVisible((HWND)CallService(MS_CLUI_GETHWND, 0, 0))) {
								CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);						
								CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)(FU_FMREDRAW | FU_FMPOS));
							}
						}
					} else {
						if(IsWindowVisible(hwnd) && count == 0)
							SetReminderFrameVisible(false);
						else if(!IsWindowVisible(hwnd) && count > 0)
							// we have reminders - show if not linked to clist or if clist is visible
							if(!options.hide_with_clist || IsWindowVisible((HWND)CallService(MS_CLUI_GETHWND, 0, 0)))
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

				SendMessage(hwnd_list, WM_SETREDRAW, (WPARAM)FALSE, 0);

				EnterCriticalSection(&list_cs);
				SendMessage(hwnd_list, LB_RESETCONTENT, 0, 0);
				copy_list(alarm_list, t1, t2);
				alarm_list.sort();
				int index = 0;
				ALARM *i;
				for(alarm_list.reset(); i = alarm_list.current(); alarm_list.next(), index++) {
					if(i->flags & (ALF_HIDDEN | ALF_SUSPENDED | ALF_NOREMINDER))
						continue;
					SendMessage(hwnd_list, LB_ADDSTRING, 0, (LPARAM)index);
				}
				LeaveCriticalSection(&list_cs);

				SendMessage(hwnd, WMU_SIZE_LIST, 0, 0);
				SendMessage(hwnd_list, WM_SETREDRAW, (WPARAM)TRUE, 0);

				if(sel != LB_ERR && sel < index) SendMessage(hwnd_list, LB_SETCURSEL, (WPARAM)sel, 0);
				if(top != LB_ERR && top < index) SendMessage(hwnd_list, LB_SETTOPINDEX, (WPARAM)top, 0);

			}
			return TRUE;
		
		case WM_TIMER:
			if(wParam == ID_FRAME_UPDATE_TIMER)
				SendMessage(hwnd, WMU_FILL_LIST, 0, 0);
			else if(wParam == ID_FRAME_SHOWHIDE_TIMER && options.hide_with_clist) { // link show/hide with clist
				// hide if we're visible and clist isn't (possible only when floating if frames are present)
				if(!IsWindowVisible((HWND)CallService(MS_CLUI_GETHWND, 0, 0)) && IsWindowVisible(hwnd)) {
					if(ServiceExists(MS_CLIST_FRAMES_SHFRAME))
						CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);
					else
						SetReminderFrameVisible(false);
				}
				// we're not visible but clist is - show depending on hide_with_clist and auto_showhide options
				if(!IsWindowVisible(hwnd) && IsWindowVisible((HWND)CallService(MS_CLUI_GETHWND, 0, 0))) {
					// if not auto show/hide, show (reminders or not) if we're not visible and the clist is
					// otherwise, show only if there are reminders
					int count = SendMessage(hwnd_list, LB_GETCOUNT, 0, 0);
					if(!options.auto_showhide || count > 0) {
						if(ServiceExists(MS_CLIST_FRAMES_SHFRAME))
							CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);
						else
							SetReminderFrameVisible(true);
					}
				}
			}
			return TRUE;

		case WM_CONTEXTMENU:
			{
				/*
				*/
				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(hwnd_list, &pt);

				EnterCriticalSection(&list_cs);
				DWORD item = SendMessage(hwnd_list, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));

				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1)), submenu = GetSubMenu(menu, 0);
				CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);
	
				if(HIWORD(item) == 0) {
					int sel = LOWORD(item);
					if(sel >= 0) {
						// one-off alarms can't be suspended
						int index = SendMessage(hwnd_list, LB_GETITEMDATA, (WPARAM)sel, 0);
						ALARM &list_alarm = alarm_list.at(index);
						copy_alarm_data(&context_menu_alarm, &list_alarm);
						if(context_menu_alarm.occurrence == OC_ONCE)
							EnableMenuItem(submenu, ID_REMINDERFRAMECONTEXT_SUSPEND, MF_BYCOMMAND | MF_GRAYED);

					}
				} else {
					EnableMenuItem(submenu, ID_REMINDERFRAMECONTEXT_SUSPEND, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(submenu, ID_REMINDERFRAMECONTEXT_EDIT, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(submenu, ID_REMINDERFRAMECONTEXT_DELETE, MF_BYCOMMAND | MF_GRAYED);
				}
				LeaveCriticalSection(&list_cs);

				//ClientToScreen(hwnd_list, &pt);
				GetCursorPos(&pt);
				
				BOOL ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
				DestroyMenu(menu);
				if(ret) PostMessage(hwnd, WM_COMMAND, ret, 0);
			}
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_REMINDERFRAMECONTEXT_OPTIONS:
					{
						OPENOPTIONSDIALOG oop;
						oop.cbSize = sizeof(oop);
						oop.pszGroup = Translate("Events");
						oop.pszPage = Translate("Alarms");
						oop.pszTab = 0;
						CallService(MS_OPT_OPENOPTIONS, 0, (LPARAM)&oop);
					}
					break;
				case ID_REMINDERFRAMECONTEXT_SUSPEND:
					if(context_menu_alarm.occurrence != OC_ONCE) {
						suspend(context_menu_alarm.id);
						PostMessage(hwnd, WMU_FILL_LIST, 0, 0);
						if(hwndOptionsDialog) {
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
					if(hwndOptionsDialog) {
						// refresh options list
						PostMessage(hwndOptionsDialog, WMU_INITOPTLIST, 0, 0);
					}
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

int ReloadFont(WPARAM wParam, LPARAM lParam) {
	DeleteObject(hFont);

	LOGFONT log_font;
	fontColour = CallService(MS_FONT_GET, (WPARAM)&font_id, (LPARAM)&log_font);
	hFont = CreateFontIndirect(&log_font);
	SendMessage(hwnd_list, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	DeleteObject(bk_brush);
	bk_brush = CreateSolidBrush(DBGetContactSettingDword(0, "Alarm", "clFrameBack", GetSysColor(COLOR_3DFACE)));
	RefreshReminderFrame();

	return 0;
}

void FixMainMenu() {
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(CLISTMENUITEM);
	if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		if(options.hide_with_clist || options.auto_showhide) {
			mi.flags = CMIM_FLAGS | CMIF_GRAYED;
		} else {
			mi.flags = CMIM_NAME | CMIM_FLAGS;

			if(ReminderFrameVisible())
				mi.pszName = Translate("Hide Reminders");
			else
				mi.pszName = Translate("Show Reminders");
		}
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuShowReminders, (LPARAM)&mi);
}

/////////////////////////
// only used when no multiwindow functionality is available
bool ReminderFrameVisible() {
	return IsWindowVisible(hwnd_frame) ? true : false;
}

void SetReminderFrameVisible(bool visible) {
	if(frame_id == -1 && hwnd_frame != 0) 
		ShowWindow(hwnd_frame, visible ? SW_SHOW : SW_HIDE);
}

int ShowHideMenuFunc(WPARAM wParam, LPARAM lParam) {
	if(ReminderFrameVisible())
		SendMessage(hwnd_frame, WM_CLOSE, 0, 0);
	else {
		ShowWindow(hwnd_frame, SW_SHOW);
	}
	FixMainMenu();
	return 0;
}
//////////////////////////////

int CreateFrame() 
{
	WNDCLASS wndclass;
	wndclass.style         = 0;
	wndclass.lpfnWndProc   = FrameWindowProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInst;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = "AlarmsFrame";
	RegisterClass(&wndclass);

	if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {

		hwnd_plugin = CreateWindow("AlarmsFrame",Translate("Alarms"), 
			WS_CHILD | WS_CLIPCHILDREN, 
			0,0,10,10, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL,hInst,NULL);

		CLISTFrame Frame = {0};
		
		Frame.cbSize=sizeof(CLISTFrame);
		Frame.name = Frame.TBname = Translate("Alarms");
		Frame.hWnd=hwnd_plugin;
		Frame.align=alBottom;
		Frame.Flags=F_VISIBLE|F_SHOWTB|F_SHOWTBTIP;
		Frame.height=30;

		frame_id=CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,0);
	} else {
		wndclass.style         = 0;//CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc   = FrameContainerWindowProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = hInst;
		wndclass.hIcon         = NULL;
		wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
		wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = "AlarmsFrameContainer";
		RegisterClass(&wndclass);

		hwnd_frame = CreateWindowEx(WS_EX_TOOLWINDOW, "AlarmsFrameContainer",Translate("Alarms"), 
			(WS_POPUPWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN) & ~WS_VISIBLE,
			0,0,200,100, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL,hInst,NULL);
			//0,0,200,100, GetDesktopWindow(), NULL,hInst,NULL);
	
		hwnd_plugin = CreateWindow("AlarmsFrame",Translate("Alarms"), 
			WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
			0,0,10,10, hwnd_frame, NULL,hInst,NULL);

		SetWindowLong(hwnd_frame, GWL_USERDATA, (LONG)hwnd_plugin);

		///////////////////////
		// create menu item
		CreateServiceFunction(MODULE "/ShowHideReminders", ShowHideMenuFunc);

		CLISTMENUITEM menu = {0};

		menu.cbSize=sizeof(menu);
		menu.flags = CMIM_ALL;

		menu.hIcon=hIconMenuShowHide;
		menu.pszName = Translate("Show Reminders");
		menu.pszService= MODULE "/ShowHideReminders";
		menu.pszPopupName = Translate("Alarms");
		menu.position = 500010000;
		hMenuShowReminders = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menu);
		/////////////////////

		if(!options.auto_showhide) {
			if(options.hide_with_clist) {
				if(IsWindowVisible((HWND)CallService(MS_CLUI_GETHWND, 0, 0))) {
					ShowWindow(hwnd_frame, SW_SHOW);
					RefreshReminderFrame();
				} else {
					ShowWindow(hwnd_frame, SW_HIDE);
				}
			} else {
				if(DBGetContactSettingByte(0, MODULE, "ReminderFrameVisible", 1) == 1) {
					ShowWindow(hwnd_frame, SW_SHOW);
					RefreshReminderFrame();
				} else {
					ShowWindow(hwnd_frame, SW_HIDE);
				}
			}
		}

		FixMainMenu();
	}

	SendMessage(hwnd_plugin, WMU_INITIALIZE, 0, 0);

	if(ServiceExists(MS_FONT_REGISTER)) {
		font_id.cbSize = sizeof(FontID);
		strncpy(font_id.group, Translate("Frames"), sizeof(font_id.group));
		strncpy(font_id.name, Translate("Alarm Reminders"), sizeof(font_id.name));
		strncpy(font_id.dbSettingsGroup, MODULE, sizeof(font_id.dbSettingsGroup));
		strncpy(font_id.prefix, "Font", sizeof(font_id.prefix));
		font_id.order = 0;
		CallService(MS_FONT_REGISTER, (WPARAM)&font_id, 0);

		framebk_colour_id.cbSize = sizeof(ColourID);
		strcpy(framebk_colour_id.dbSettingsGroup, MODULE);
		strcpy(framebk_colour_id.group, Translate("Frames"));
		strcpy(framebk_colour_id.name, Translate("Alarm Reminders"));
		strcpy(framebk_colour_id.setting, "clFrameBack");
		framebk_colour_id.defcolour = GetSysColor(COLOR_3DFACE);
		framebk_colour_id.flags = 0;
		framebk_colour_id.order = 0;
		CallService(MS_COLOUR_REGISTER, (WPARAM)&framebk_colour_id, 0);

		LOGFONT log_font;
		fontColour = CallService(MS_FONT_GET, (WPARAM)&font_id, (LPARAM)&log_font);
		hFont = CreateFontIndirect(&log_font);
		SendMessage(hwnd_list, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	
		HookEvent(ME_FONT_RELOAD, ReloadFont);
		
	} else {
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);
		lf.lfHeight = 10;

		HDC hdc = GetDC(0);
		lf.lfHeight = -MulDiv(lf.lfHeight,GetDeviceCaps(hdc, LOGPIXELSY), 72);
		ReleaseDC(0, hdc);
		
		hFont = CreateFontIndirect(&lf);

		//fontColour = GetSysColor(COLOR_WINDOWTEXT);
		// match clist 'normal contact' colour if no font service
		fontColour = (COLORREF)DBGetContactSettingDword(0, "CLC", "Font0Col", GetSysColor(COLOR_WINDOWTEXT));
	}

	// create the brush used for the background in the absence of clist_modern skinning features - match clist
	bk_brush = CreateSolidBrush(DBGetContactSettingDword(0, "Alarm", "clFrameBack", GetSysColor(COLOR_3DFACE)));

	SendMessage(hwnd_list, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	return 0;
}

void RefreshReminderFrame() {
	SendMessage(hwnd_plugin, WMU_FILL_LIST, 0, 0);

	if(frame_id == -1) {
		InvalidateRect(hwnd_frame, 0, TRUE);
	} else
		InvalidateRect(hwnd_plugin, 0, TRUE);

}

void InitFrames()
{
	InitializeCriticalSection(&list_cs);
	CreateFrame();
}

void DeinitFrames()
{
	if(ServiceExists(MS_CLIST_FRAMES_REMOVEFRAME)) {
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)frame_id, 0);
	}
	DestroyWindow(hwnd_plugin);
	if(hwnd_frame) DestroyWindow(hwnd_frame);

	DeleteObject(bk_brush);

	DeleteCriticalSection(&list_cs);
}

