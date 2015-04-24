#include "stdafx.h"
#include "alarm_win.h"

#define ID_TIMER_SOUND				10101
#define SOUND_REPEAT_PERIOD			5000	// milliseconds
#define SPEACH_REPEAT_PERIOD		15000	// milliseconds
HANDLE hAlarmWindowList = 0;

FontIDT title_font_id, window_font_id;
ColourIDT bk_colour_id;
HFONT hTitleFont = 0, hWindowFont = 0;
COLORREF title_font_colour, window_font_colour;
HBRUSH hBackgroundBrush = 0;

#define WMU_SETFONTS		(WM_USER + 61)
#define WMU_REFRESH			(WM_USER + 62)
#define WMU_ADDSNOOZER		(WM_USER + 63)

int win_num = 0;
typedef struct WindowData_tag {
	ALARM *alarm;
	POINT p;
	bool moving;
	int win_num;
} WindowData;

void SetAlarmWinOptions()
{
	WindowList_Broadcast(hAlarmWindowList, WMU_SETOPT, IDC_SNOOZE, 0);
}

INT_PTR CALLBACK DlgProcAlarm(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG: 
		TranslateDialogDefault( hwndDlg );
		{
			Utils_RestoreWindowPositionNoSize(hwndDlg, 0, MODULE, "Notify");
			SetFocus(GetDlgItem(hwndDlg, IDC_SNOOZE));

			WindowData *wd = new WindowData;
			wd->moving = false;
			wd->alarm = 0;
			wd->win_num = win_num++;

			if (wd->win_num > 0) {
				RECT r;
				GetWindowRect(hwndDlg, &r);
				r.top += 20;
				r.left += 20;

				SetWindowPos(hwndDlg, 0, r.left, r.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
				Utils_SaveWindowPosition(hwndDlg, 0, MODULE, "Notify");
			}

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)wd);

			// options
			SendMessage(hwndDlg, WMU_SETOPT, 0, 0);
		
			// fonts
			SendMessage(hwndDlg, WMU_SETFONTS, 0, 0);
		}
		return FALSE;

	case WMU_REFRESH:
		InvalidateRect(hwndDlg, 0, TRUE);
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
				return (BOOL)hBackgroundBrush;
			}
		}
		break;
	case WM_CTLCOLORDLG:
		if (hBackgroundBrush)
			return (BOOL)hBackgroundBrush;
		break;
	case WMU_SETFONTS:
		// fonts
		if (hWindowFont)
			SendMessage(hwndDlg, WM_SETFONT, (WPARAM)hWindowFont, TRUE);
		if (hTitleFont)
			SendDlgItemMessage(hwndDlg, IDC_TITLE, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

		if (hBackgroundBrush) {
			SetClassLong(hwndDlg, GCLP_HBRBACKGROUND, (LONG)hBackgroundBrush);
			InvalidateRect(hwndDlg, 0, TRUE);
		}
		return TRUE;

	case WMU_SETOPT:
		{
			Options *opt;
			if (lParam) opt = (Options *)lParam;
			else opt = &options;
			
			// round corners
			if (opt->aw_roundcorners) {
				HRGN hRgn1;
				RECT r;
				int v,h;
				int w=10;
				GetWindowRect(hwndDlg,&r);
				h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
				v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
				h=(h<v)?h:v;
				hRgn1=CreateRoundRectRgn(0,0,(r.right-r.left+1),(r.bottom-r.top+1),h,h);
				SetWindowRgn(hwndDlg,hRgn1,1);
			}
			else {
				HRGN hRgn1;
				RECT r;
				int v,h;
				int w=10;
				GetWindowRect(hwndDlg,&r);
				h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
				v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
				h=(h<v)?h:v;
				hRgn1=CreateRectRgn(0,0,(r.right-r.left+1),(r.bottom-r.top+1));
				SetWindowRgn(hwndDlg,hRgn1,1);
			}
			// transparency
		
#ifdef WS_EX_LAYERED 
			SetWindowLongPtr(hwndDlg, GWL_EXSTYLE, GetWindowLongPtr(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
#endif
#ifdef LWA_ALPHA
			SetLayeredWindowAttributes(hwndDlg, RGB(0,0,0), (int)((100 - opt->aw_trans) / 100.0 * 255), LWA_ALPHA);
#endif
		}
		return TRUE;

	case WMU_SETALARM:
		{
			ALARM *data = (ALARM *)lParam;
			SetWindowText(hwndDlg, data->szTitle);
			HWND hw = GetDlgItem(hwndDlg, IDC_TITLE);
			SetWindowText(hw, data->szTitle);

			SetDlgItemText(hwndDlg, IDC_ED_DESC, data->szDesc);
			((WindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA))->alarm = data;

			if (data->action & AAF_SOUND && options.loop_sound) {
				if (data->sound_num <= 3)
					SetTimer(hwndDlg, ID_TIMER_SOUND, SOUND_REPEAT_PERIOD, 0);	
				else if (data->sound_num == 4) 
					SetTimer(hwndDlg, ID_TIMER_SOUND, SPEACH_REPEAT_PERIOD, 0);	
			}

			hw = GetDlgItem(hwndDlg, IDC_SNOOZE);
			EnableWindow(hw, !(data->flags & ALF_NOSNOOZE));
			ShowWindow(hw, (data->flags & ALF_NOSNOOZE) ? SW_HIDE : SW_SHOWNA);
		}
		return TRUE;

	case WMU_FAKEALARM:
		{
			SetWindowText(hwndDlg, TranslateT("Example alarm"));
			SetDlgItemText(hwndDlg, IDC_TITLE, TranslateT("Example alarm"));
			SetDlgItemText(hwndDlg, IDC_ED_DESC, TranslateT("Some example text. Example, example, example."));
		}
		return TRUE;

	case WM_TIMER: 
		if (wParam == ID_TIMER_SOUND) {
			WindowData *dw = (WindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (dw) {
				ALARM *data = dw->alarm;
				if (data && data->action & AAF_SOUND) {
					if (data->sound_num <= 3) {
						char buff[128];
						mir_snprintf(buff, SIZEOF(buff), "Triggered%d", data->sound_num);
						SkinPlaySound(buff);
					}
					else if (data->sound_num == 4) {
						if (data->szTitle != NULL && data->szTitle[0] != '\0') {
							if (ServiceExists("Speak/Say")) {
								CallService("Speak/Say", 0, (LPARAM)data->szTitle);
							}
						}
					}
				}
			}
		}
		return TRUE;

	case WM_MOVE:
		Utils_SaveWindowPosition(hwndDlg, 0, MODULE, "Notify");
		break;

	case WMU_ADDSNOOZER:
		{
			WindowData *wd = (WindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
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
		}
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDCANCEL:  // no button - esc pressed
			case IDOK:		// space?
			case IDC_SNOOZE:
				SendMessage(hwndDlg, WMU_ADDSNOOZER, (WPARAM)options.snooze_minutes, 0);
				//drop through
			case IDC_DISMISS:
				{
					WindowData *window_data = (WindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					KillTimer(hwndDlg, ID_TIMER_SOUND);
					if (window_data) {
						if (window_data->alarm) {
							free_alarm_data(window_data->alarm);
							delete window_data->alarm;
						}
						delete window_data;
					}
					SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);

					win_num--;
					//EndDialog(hwndDlg, IDOK); // not modal!
					WindowList_Remove(hAlarmWindowList, hwndDlg);
					DestroyWindow(hwndDlg);
				}
				break;
			case IDC_SNOOZELIST:
				{
					POINT pt, pt_rel;
					GetCursorPos(&pt);				
					pt_rel = pt;
					ScreenToClient(hwndDlg,&pt_rel);

					HMENU hMenu = CreatePopupMenu();
					MENUITEMINFO mmi = {0};
					mmi.cbSize = sizeof(mmi);
					mmi.fMask = MIIM_ID | MIIM_STRING;

#define AddItem(x)							\
		mmi.wID++;							\
		mmi.dwTypeData = TranslateT(x);		\
		mmi.cch = ( UINT )_tcslen(mmi.dwTypeData);	\
		InsertMenuItem(hMenu, mmi.wID, FALSE, &mmi);

					AddItem(LPGEN("5 mins"));
					AddItem(LPGEN("15 mins"));
					AddItem(LPGEN("30 mins"));
					AddItem(LPGEN("1 hour"));
					AddItem(LPGEN("1 day"));
					AddItem(LPGEN("1 week"));

					TPMPARAMS tpmp = {0};
					tpmp.cbSize	= sizeof(tpmp);
					LRESULT ret = (LRESULT)TrackPopupMenuEx(hMenu, TPM_RETURNCMD, pt.x, pt.y, hwndDlg, &tpmp);
					switch(ret) {
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
				}
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

			WindowData *window_data = (WindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (!window_data->moving)
				window_data->moving = true;
			else {
				RECT r;
				GetWindowRect(hwndDlg, &r);
				
				SetWindowPos(hwndDlg, 0, r.left + (newp.x - window_data->p.x), r.top + (newp.y - window_data->p.y), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			}
			window_data->p.x = newp.x;
			window_data->p.y = newp.y;			
		}
		else {
			ReleaseCapture();
			WindowData *window_data = (WindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			window_data->moving = false;
		}
		return TRUE;
	}

	return FALSE;
}

int ReloadFonts(WPARAM wParam, LPARAM lParam)
{
	LOGFONT log_font;
	title_font_colour = CallService(MS_FONT_GETT, (WPARAM)&title_font_id, (LPARAM)&log_font);
	DeleteObject(hTitleFont);
	hTitleFont = CreateFontIndirect(&log_font);

	window_font_colour = CallService(MS_FONT_GETT, (WPARAM)&window_font_id, (LPARAM)&log_font);
	DeleteObject(hWindowFont);
	hWindowFont = CreateFontIndirect(&log_font);

	COLORREF bkCol = CallService(MS_COLOUR_GETT, (WPARAM)&bk_colour_id, 0);
	DeleteObject(hBackgroundBrush);
	hBackgroundBrush = CreateSolidBrush(bkCol);

	WindowList_Broadcast(hAlarmWindowList, WMU_REFRESH, 0, 0);
	return 0;
}

int AlarmWinModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	title_font_id.cbSize = sizeof(FontIDT);
	_tcscpy(title_font_id.group, LPGENT("Alarms"));
	_tcscpy(title_font_id.name, LPGENT("Title"));
	strcpy(title_font_id.dbSettingsGroup, MODULE);
	strcpy(title_font_id.prefix, "FontTitle");
	_tcscpy(title_font_id.backgroundGroup,LPGENT("Alarms"));
	_tcscpy(title_font_id.backgroundName,LPGENT("Background"));
	title_font_id.flags = 0;
	title_font_id.order = 0;
	FontRegisterT(&title_font_id);

	window_font_id.cbSize = sizeof(FontIDT);
	_tcscpy(window_font_id.group, LPGENT("Alarms"));
	_tcscpy(window_font_id.name, LPGENT("Window"));
	strcpy(window_font_id.dbSettingsGroup, MODULE);
	strcpy(window_font_id.prefix, "FontWindow");
	_tcscpy(window_font_id.backgroundGroup,LPGENT("Alarms"));
	_tcscpy(window_font_id.backgroundName,LPGENT("Background"));
	window_font_id.flags = 0;
	window_font_id.order = 1;
	FontRegisterT(&window_font_id);

	bk_colour_id.cbSize = sizeof(ColourIDT);
	strcpy(bk_colour_id.dbSettingsGroup, MODULE);
	_tcscpy(bk_colour_id.group, LPGENT("Alarms"));
	_tcscpy(bk_colour_id.name, LPGENT("Background"));
	strcpy(bk_colour_id.setting, "BkColour");
	bk_colour_id.defcolour = GetSysColor(COLOR_3DFACE);
	bk_colour_id.flags = 0;
	bk_colour_id.order = 0;

	ColourRegisterT(&bk_colour_id);

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

