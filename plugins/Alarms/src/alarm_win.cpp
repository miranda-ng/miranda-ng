#include "stdafx.h"

#define ID_TIMER_SOUND				10101
#define SOUND_REPEAT_PERIOD			5000	// milliseconds
#define SPEACH_REPEAT_PERIOD		15000	// milliseconds
MWindowList hAlarmWindowList = nullptr;

FontIDW title_font_id, window_font_id;
ColourIDW bk_colour_id;
HFONT hTitleFont = nullptr, hWindowFont = nullptr;
COLORREF title_font_colour, window_font_colour;
HBRUSH hBackgroundBrush = nullptr;

#define WMU_SETFONTS		(WM_USER + 61)
#define WMU_REFRESH			(WM_USER + 62)
#define WMU_ADDSNOOZER		(WM_USER + 63)

int win_num = 0;
struct WindowData
{
	ALARM *alarm;
	POINT p;
	bool moving;
	int win_num;
};

void SetAlarmWinOptions()
{
	WindowList_Broadcast(hAlarmWindowList, WMU_SETOPT, IDC_SNOOZE, 0);
}

INT_PTR CALLBACK DlgProcAlarm(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WindowData *wd = (WindowData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		Utils_RestoreWindowPositionNoSize(hwndDlg, 0, MODULENAME, "Notify");
		SetFocus(GetDlgItem(hwndDlg, IDC_SNOOZE));

		wd = new WindowData;
		wd->moving = false;
		wd->alarm = nullptr;
		wd->win_num = win_num++;

		if (wd->win_num > 0) {
			RECT r;
			GetWindowRect(hwndDlg, &r);
			r.top += 20;
			r.left += 20;

			SetWindowPos(hwndDlg, nullptr, r.left, r.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			Utils_SaveWindowPosition(hwndDlg, 0, MODULENAME, "Notify");
		}
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)wd);

		// options
		SendMessage(hwndDlg, WMU_SETOPT, 0, 0);

		// fonts
		SendMessage(hwndDlg, WMU_SETFONTS, 0, 0);
		return FALSE;

	case WMU_REFRESH:
		InvalidateRect(hwndDlg, nullptr, TRUE);
		return TRUE;

	case WM_CTLCOLORSTATIC:
		{
			HDC hdc = (HDC)wParam;
			HWND hwndCtrl = (HWND)lParam;

			if (hBackgroundBrush) {
				if (hTitleFont && GetDlgItem(hwndDlg, IDC_TITLE) == hwndCtrl)
					SetTextColor(hdc, title_font_colour);
				else if (hWindowFont)
					SetTextColor(hdc, window_font_colour);

				SetBkMode(hdc, TRANSPARENT);
				return (INT_PTR)hBackgroundBrush;
			}
		}
		break;

	case WM_CTLCOLORDLG:
		if (hBackgroundBrush)
			return (INT_PTR)hBackgroundBrush;
		break;

	case WMU_SETFONTS:
		// fonts
		if (hWindowFont)
			SendMessage(hwndDlg, WM_SETFONT, (WPARAM)hWindowFont, TRUE);
		if (hTitleFont)
			SendDlgItemMessage(hwndDlg, IDC_TITLE, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

		if (hBackgroundBrush) {
			SetClassLongPtr(hwndDlg, GCLP_HBRBACKGROUND, (LONG_PTR)hBackgroundBrush);
			InvalidateRect(hwndDlg, nullptr, TRUE);
		}
		return TRUE;

	case WMU_SETOPT:
		Options *opt;
		if (lParam)
			opt = (Options*)lParam;
		else
			opt = &options;

		// round corners
		if (opt->aw_roundcorners) {
			HRGN hRgn1;
			RECT r;
			int w = 10;
			GetWindowRect(hwndDlg, &r);
			int h = (r.right - r.left) > (w * 2) ? w : (r.right - r.left);
			int v = (r.bottom - r.top) > (w * 2) ? w : (r.bottom - r.top);
			h = (h < v) ? h : v;
			hRgn1 = CreateRoundRectRgn(0, 0, (r.right - r.left + 1), (r.bottom - r.top + 1), h, h);
			SetWindowRgn(hwndDlg, hRgn1, 1);
		}
		else {
			HRGN hRgn1;
			RECT r;
			int w = 10;
			GetWindowRect(hwndDlg, &r);
			int h = (r.right - r.left) > (w * 2) ? w : (r.right - r.left);
			int v = (r.bottom - r.top) > (w * 2) ? w : (r.bottom - r.top);
			h = (h < v) ? h : v;
			hRgn1 = CreateRectRgn(0, 0, (r.right - r.left + 1), (r.bottom - r.top + 1));
			SetWindowRgn(hwndDlg, hRgn1, 1);
		}
		// transparency

		#ifdef WS_EX_LAYERED 
			SetWindowLongPtr(hwndDlg, GWL_EXSTYLE, GetWindowLongPtr(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
		#endif
		#ifdef LWA_ALPHA
			SetLayeredWindowAttributes(hwndDlg, RGB(0, 0, 0), (int)((100 - opt->aw_trans) / 100.0 * 255), LWA_ALPHA);
		#endif
		return TRUE;

	case WMU_SETALARM:
		{
			ALARM *data = (ALARM *)lParam;
			SetWindowText(hwndDlg, data->szTitle);
			SetWindowText(GetDlgItem(hwndDlg, IDC_TITLE), data->szTitle);

			SetDlgItemText(hwndDlg, IDC_ED_DESC, data->szDesc);
			wd->alarm = data;

			if (data->action & AAF_SOUND && options.loop_sound) {
				if (data->sound_num <= 3)
					SetTimer(hwndDlg, ID_TIMER_SOUND, SOUND_REPEAT_PERIOD, nullptr);
				else if (data->sound_num == 4)
					SetTimer(hwndDlg, ID_TIMER_SOUND, SPEACH_REPEAT_PERIOD, nullptr);
			}

			HWND hw = GetDlgItem(hwndDlg, IDC_SNOOZE);
			EnableWindow(hw, !(data->flags & ALF_NOSNOOZE));
			ShowWindow(hw, (data->flags & ALF_NOSNOOZE) ? SW_HIDE : SW_SHOWNA);
		}
		return TRUE;

	case WMU_FAKEALARM:
		SetWindowText(hwndDlg, TranslateT("Example alarm"));
		SetDlgItemText(hwndDlg, IDC_TITLE, TranslateT("Example alarm"));
		SetDlgItemText(hwndDlg, IDC_ED_DESC, TranslateT("Some example text. Example, example, example."));
		return TRUE;

	case WM_TIMER:
		if (wParam == ID_TIMER_SOUND && wd) {
			ALARM *data = wd->alarm;
			if (data && data->action & AAF_SOUND) {
				if (data->sound_num <= 3) {
					char buff[128];
					mir_snprintf(buff, "Triggered%d", data->sound_num);
					Skin_PlaySound(buff);
				}
				else if (data->sound_num == 4) {
					if (data->szTitle != nullptr && data->szTitle[0] != '\0') {
						if (ServiceExists("Speak/Say")) {
							CallService("Speak/Say", 0, (LPARAM)data->szTitle);
						}
					}
				}
			}
		}
		return TRUE;

	case WM_MOVE:
		Utils_SaveWindowPosition(hwndDlg, 0, MODULENAME, "Notify");
		break;

	case WMU_ADDSNOOZER:
		if (wd) {
			ALARM *data = wd->alarm;
			if (data) {
				// add snooze minutes to current time
				FILETIME ft;
				GetLocalTime(&data->time);
				SystemTimeToFileTime(&data->time, &ft);

				ULARGE_INTEGER uli;
				uli.LowPart = ft.dwLowDateTime;
				uli.HighPart = ft.dwHighDateTime;

				// there are 10000000 100-nanosecond blocks in a second...
				uli.QuadPart += mult.QuadPart * (int)(wParam);

				ft.dwHighDateTime = uli.HighPart;
				ft.dwLowDateTime = uli.LowPart;

				FileTimeToSystemTime(&ft, &data->time);

				data->occurrence = OC_ONCE;
				data->snoozer = true;
				data->flags = data->flags & ~ALF_NOSTARTUP;

				data->id = next_alarm_id++;

				append_to_list(data);
			}
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case IDCANCEL:  // no button - esc pressed
			case IDOK:		// space?
			case IDC_SNOOZE:
				SendMessage(hwndDlg, WMU_ADDSNOOZER, (WPARAM)options.snooze_minutes, 0);
				//drop through

			case IDC_DISMISS:
				KillTimer(hwndDlg, ID_TIMER_SOUND);
				if (wd) {
					if (wd->alarm) {
						free_alarm_data(wd->alarm);
						delete wd->alarm;
					}
					delete wd;
				}
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);

				win_num--;
				WindowList_Remove(hAlarmWindowList, hwndDlg);
				DestroyWindow(hwndDlg);
				break;

			case IDC_SNOOZELIST:
				POINT pt, pt_rel;
				GetCursorPos(&pt);
				pt_rel = pt;
				ScreenToClient(hwndDlg, &pt_rel);

				HMENU hMenu = CreatePopupMenu();
				MENUITEMINFO mii = { 0 };
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_STRING;

				#define AddItem(x) \
					mii.wID++; \
					mii.dwTypeData = TranslateW(x); \
					mii.cch = ( UINT )mir_wstrlen(mii.dwTypeData); \
					InsertMenuItem(hMenu, mii.wID, FALSE, &mii);

				AddItem(LPGENW("5 mins"));
				AddItem(LPGENW("15 mins"));
				AddItem(LPGENW("30 mins"));
				AddItem(LPGENW("1 hour"));
				AddItem(LPGENW("1 day"));
				AddItem(LPGENW("1 week"));

				TPMPARAMS tpmp = { 0 };
				tpmp.cbSize = sizeof(tpmp);
				LRESULT ret = (LRESULT)TrackPopupMenuEx(hMenu, TPM_RETURNCMD, pt.x, pt.y, hwndDlg, &tpmp);
				switch (ret) {
					case 0: DestroyMenu(hMenu); return 0; // dismis menu
					case 1: SendMessage(hwndDlg, WMU_ADDSNOOZER, (WPARAM)5, 0); break;
					case 2: SendMessage(hwndDlg, WMU_ADDSNOOZER, (WPARAM)15, 0); break;
					case 3: SendMessage(hwndDlg, WMU_ADDSNOOZER, (WPARAM)30, 0); break;
					case 4: SendMessage(hwndDlg, WMU_ADDSNOOZER, (WPARAM)60, 0); break;
					case 5: SendMessage(hwndDlg, WMU_ADDSNOOZER, (WPARAM)(60 * 24), 0); break;
					case 6: SendMessage(hwndDlg, WMU_ADDSNOOZER, (WPARAM)(60 * 24 * 7), 0); break;
				}

				DestroyMenu(hMenu);

				SendMessage(hwndDlg, WM_COMMAND, IDC_DISMISS, 0);
				break;
			}
		}
		return TRUE;

	case WM_MOUSEMOVE:
		if (wParam & MK_LBUTTON) {
			SetCapture(hwndDlg);

			POINT newp;
			newp.x = (short)LOWORD(lParam);
			newp.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &newp);

			if (!wd->moving)
				wd->moving = true;
			else {
				RECT r;
				GetWindowRect(hwndDlg, &r);

				SetWindowPos(hwndDlg, nullptr, r.left + (newp.x - wd->p.x), r.top + (newp.y - wd->p.y), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			}
			wd->p.x = newp.x;
			wd->p.y = newp.y;
		}
		else {
			ReleaseCapture();
			wd->moving = false;
		}
		return TRUE;
	}

	return FALSE;
}

int ReloadFonts(WPARAM, LPARAM)
{
	LOGFONT log_font;
	title_font_colour = Font_GetW(title_font_id, &log_font);
	DeleteObject(hTitleFont);
	hTitleFont = CreateFontIndirect(&log_font);

	window_font_colour = Font_GetW(window_font_id, &log_font);
	DeleteObject(hWindowFont);
	hWindowFont = CreateFontIndirect(&log_font);

	COLORREF bkCol = Colour_GetW(bk_colour_id);
	DeleteObject(hBackgroundBrush);
	hBackgroundBrush = CreateSolidBrush(bkCol);

	WindowList_Broadcast(hAlarmWindowList, WMU_REFRESH, 0, 0);
	return 0;
}

int AlarmWinModulesLoaded(WPARAM, LPARAM)
{
	wcsncpy_s(title_font_id.group, LPGENW("Alarms"), _TRUNCATE);
	wcsncpy_s(title_font_id.name, LPGENW("Title"), _TRUNCATE);
	strncpy_s(title_font_id.dbSettingsGroup, MODULENAME, _TRUNCATE);
	strncpy_s(title_font_id.setting, "FontTitle", _TRUNCATE);
	wcsncpy_s(title_font_id.backgroundGroup, LPGENW("Alarms"), _TRUNCATE);
	wcsncpy_s(title_font_id.backgroundName, LPGENW("Background"), _TRUNCATE);
	title_font_id.flags = 0;
	title_font_id.order = 0;
	g_plugin.addFont(&title_font_id);

	wcsncpy_s(window_font_id.group, LPGENW("Alarms"), _TRUNCATE);
	wcsncpy_s(window_font_id.name, LPGENW("Window"), _TRUNCATE);
	strncpy_s(window_font_id.dbSettingsGroup, MODULENAME, _TRUNCATE);
	strncpy_s(window_font_id.setting, "FontWindow", _TRUNCATE);
	wcsncpy_s(window_font_id.backgroundGroup, LPGENW("Alarms"), _TRUNCATE);
	wcsncpy_s(window_font_id.backgroundName, LPGENW("Background"), _TRUNCATE);
	window_font_id.flags = 0;
	window_font_id.order = 1;
	g_plugin.addFont(&window_font_id);

	strncpy_s(bk_colour_id.dbSettingsGroup, MODULENAME, _TRUNCATE);
	wcsncpy_s(bk_colour_id.group, LPGENW("Alarms"), _TRUNCATE);
	wcsncpy_s(bk_colour_id.name, LPGENW("Background"), _TRUNCATE);
	strncpy_s(bk_colour_id.setting, "BkColour", _TRUNCATE);
	bk_colour_id.defcolour = GetSysColor(COLOR_3DFACE);
	bk_colour_id.order = 0;

	g_plugin.addColor(&bk_colour_id);

	ReloadFonts(0, 0);
	HookEvent(ME_FONT_RELOAD, ReloadFonts);
	return 0;
}

void InitAlarmWin()
{
	hAlarmWindowList = WindowList_Create();

	HookEvent(ME_SYSTEM_MODULESLOADED, AlarmWinModulesLoaded);
}

void DeinitAlarmWin()
{
	WindowList_Broadcast(hAlarmWindowList, WM_COMMAND, IDC_SNOOZE, 0);
	WindowList_Destroy(hAlarmWindowList);

	if (hBackgroundBrush) DeleteObject(hBackgroundBrush);
	if (hTitleFont) DeleteObject(hTitleFont);
	if (hWindowFont) DeleteObject(hWindowFont);
}
