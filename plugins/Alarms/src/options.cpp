#include "stdafx.h"
#include "options.h"

Options options;

#define		WMU_OPTSETALARM		(WM_USER + 10)
#define		WMU_UPDATETIME		(WM_USER + 11)
#define		WMU_SETTIME			(WM_USER + 12)
#define		WMU_MYDESTROY		(WM_USER + 13)

// defined in header
//#define		WMU_INITOPTLIST		(WM_USER + 20)

typedef struct AddEditParam_tag {
	ALARM *alarm_ptr;
	BOOL edit;
	BOOL self_add;
} AddEditParam;

HGENMENU hMainMenuItem = 0;

// store options dialog window handle statically so it can be refreshed by non-modal add alarm dialog
HWND hwndOptionsDialog = 0;

static INT_PTR CALLBACK DlgProcAddEdit(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ALARM *add_edit_alarm = (ALARM *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	HWND hw;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		SendDlgItemMessage(hwndDlg, IDC_DAY, CB_INSERTSTRING, 0, (LPARAM)TranslateT("Sunday"));
		SendDlgItemMessage(hwndDlg, IDC_DAY, CB_INSERTSTRING, 1, (LPARAM)TranslateT("Monday"));
		SendDlgItemMessage(hwndDlg, IDC_DAY, CB_INSERTSTRING, 2, (LPARAM)TranslateT("Tuesday"));
		SendDlgItemMessage(hwndDlg, IDC_DAY, CB_INSERTSTRING, 3, (LPARAM)TranslateT("Wednesday"));
		SendDlgItemMessage(hwndDlg, IDC_DAY, CB_INSERTSTRING, 4, (LPARAM)TranslateT("Thursday"));
		SendDlgItemMessage(hwndDlg, IDC_DAY, CB_INSERTSTRING, 5, (LPARAM)TranslateT("Friday"));
		SendDlgItemMessage(hwndDlg, IDC_DAY, CB_INSERTSTRING, 6, (LPARAM)TranslateT("Saturday"));
		SendDlgItemMessage(hwndDlg, IDC_DAY, CB_SETCURSEL, 0, 0);
		{
			TCHAR buff[10];
			for(int i = 1; i <= 31; i++)
				SendDlgItemMessage(hwndDlg, IDC_DAYNUM, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)_itot(i, buff, 10));
			
			SendDlgItemMessage(hwndDlg, IDC_DAYNUM, CB_SETCURSEL, 0, 0);
			{
				HWND hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, FALSE);
			}

			if (!ServiceExists("Speak/Say"))
				EnableWindow(GetDlgItem(hwndDlg, IDC_RAD_SPK), FALSE);

			AddEditParam *param = (AddEditParam *)lParam;
			SendMessage(hwndDlg, WMU_OPTSETALARM, (WPARAM)param->edit, (LPARAM)param->alarm_ptr);

			// use invisible checkbox to store 'self_add' setting - naughty hack
			CheckDlgButton(hwndDlg, IDC_CHK_INVIS, param->self_add ? BST_CHECKED : BST_UNCHECKED);

			delete param;

			Utils_RestoreWindowPositionNoSize(hwndDlg, 0, MODULE, "AddEdit");

			if (GetDlgCtrlID((HWND)wParam) != IDC_TITLE) { 
				SetFocus(GetDlgItem(hwndDlg, IDC_TITLE)); 
				return FALSE; 
			}
		}
		return TRUE;

	case WMU_OPTSETALARM:
		{
			BOOL edit = (BOOL)wParam;
			add_edit_alarm = (ALARM *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)add_edit_alarm);

			if (edit) {
				SetDlgItemText(hwndDlg, IDC_TITLE, add_edit_alarm->szTitle);
				SetDlgItemText(hwndDlg, IDC_DESC, add_edit_alarm->szDesc);
				switch(add_edit_alarm->occurrence) {
				case OC_DAILY:
					CheckDlgButton(hwndDlg, IDC_RAD_DAILY, BST_CHECKED);
					hw = GetDlgItem(hwndDlg, IDC_BTN_CAL);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DATE);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DAY);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
					EnableWindow(hw, TRUE);
					break;

				case OC_WEEKDAYS:
					CheckDlgButton(hwndDlg, IDC_RAD_WEEKDAYS, BST_CHECKED);
					hw = GetDlgItem(hwndDlg, IDC_BTN_CAL);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DATE);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DAY);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_RELMIN);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
					EnableWindow(hw, TRUE);
					break;
	
				case OC_ONCE:
					CheckDlgButton(hwndDlg, IDC_RAD_ONCE, BST_CHECKED);
					hw = GetDlgItem(hwndDlg, IDC_DAY);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
					EnableWindow(hw, FALSE);
					break;
	
				case OC_WEEKLY:
					CheckDlgButton(hwndDlg, IDC_RAD_WEEKLY, BST_CHECKED);
					hw = GetDlgItem(hwndDlg, IDC_BTN_CAL);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DATE);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_RELMIN);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
					EnableWindow(hw, TRUE);
					break;
			
				case OC_MONTHLY:
					CheckDlgButton(hwndDlg, IDC_RAD_MONTHLY, BST_CHECKED);
					hw = GetDlgItem(hwndDlg, IDC_BTN_CAL);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DATE);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DAY);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_RELMIN);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
					EnableWindow(hw, TRUE);
					break;
	
				case OC_YEARLY:
					CheckDlgButton(hwndDlg, IDC_RAD_YEARLY, BST_CHECKED);
					hw = GetDlgItem(hwndDlg, IDC_DAY);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
					EnableWindow(hw, TRUE);
					break;
				}
				
				CheckDlgButton(hwndDlg, IDC_CHK_SUSPEND, (add_edit_alarm->flags & ALF_SUSPENDED) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_CHK_NOSTARTUP, (add_edit_alarm->flags & ALF_NOSTARTUP) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_CHK_NOREMINDER, (add_edit_alarm->flags & ALF_NOREMINDER) ? BST_CHECKED : BST_UNCHECKED);
				
				SendDlgItemMessage(hwndDlg, IDC_DAY, CB_SETCURSEL, add_edit_alarm->time.wDayOfWeek, 0);
				SendDlgItemMessage(hwndDlg, IDC_TIME, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&add_edit_alarm->time);
				SendDlgItemMessage(hwndDlg, IDC_DATE, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&add_edit_alarm->time);
				SendDlgItemMessage(hwndDlg, IDC_DAYNUM, CB_SETCURSEL, add_edit_alarm->time.wDay - 1, 0);
				SetDlgItemInt(hwndDlg, IDC_RELMIN, MinutesInFuture(add_edit_alarm->time, add_edit_alarm->occurrence), FALSE);

				if (add_edit_alarm->action & AAF_SOUND) {
					CheckDlgButton(hwndDlg, IDC_CHK_ASOUND, BST_CHECKED);
					switch(add_edit_alarm->sound_num) {
						case 1:
							CheckDlgButton(hwndDlg, IDC_RAD_SND1, BST_CHECKED);
							break;
						case 2:
							CheckDlgButton(hwndDlg, IDC_RAD_SND2, BST_CHECKED);
							break;
						case 3:
							CheckDlgButton(hwndDlg, IDC_RAD_SND3, BST_CHECKED);
							break;
						case 4:
							if (!ServiceExists("Speak/Say")) {
								add_edit_alarm->sound_num = 1;
								CheckDlgButton(hwndDlg, IDC_RAD_SND1, BST_CHECKED);
							} else
								CheckDlgButton(hwndDlg, IDC_RAD_SPK, BST_CHECKED);
							
							break;
					}
				} 
				else {
					HWND hw = GetDlgItem(hwndDlg, IDC_RAD_SND1);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_RAD_SND2);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_RAD_SND3);
					EnableWindow(hw, FALSE);
				}
				if (add_edit_alarm->action & AAF_POPUP)
					CheckDlgButton(hwndDlg, IDC_CHK_APOPUP, BST_CHECKED);
				if (add_edit_alarm->action & AAF_COMMAND) {
					CheckDlgButton(hwndDlg, IDC_CHK_ACOMMAND, BST_CHECKED);
					SetDlgItemText(hwndDlg, IDC_ED_COMMAND, add_edit_alarm->szCommand);
					SetDlgItemText(hwndDlg, IDC_ED_PARAMS, add_edit_alarm->szCommandParams);
				}
				else {
					HWND hw = GetDlgItem(hwndDlg, IDC_ED_COMMAND);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_ED_PARAMS);
					EnableWindow(hw, FALSE);
				}
			}
			else {
				SYSTEMTIME now;
				GetPluginTime(&now);
				
				// set time to 10 mins in future to help prevent 'alarm in past' error
				TimeForMinutesInFuture(10, &now);
				
				CheckDlgButton(hwndDlg, IDC_RAD_ONCE, BST_CHECKED);
				HWND hw = GetDlgItem(hwndDlg, IDC_DAY);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
				EnableWindow(hw, FALSE);
				SendDlgItemMessage(hwndDlg, IDC_DAY, CB_SETCURSEL, now.wDayOfWeek, 0);
				SendDlgItemMessage(hwndDlg, IDC_TIME, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&now);
				SendDlgItemMessage(hwndDlg, IDC_DATE, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&now);
				SendDlgItemMessage(hwndDlg, IDC_DAYNUM, CB_SETCURSEL, now.wDay - 1, 0);
				SetDlgItemInt(hwndDlg, IDC_RELMIN, MinutesInFuture(now, OC_ONCE), FALSE);

				CheckDlgButton(hwndDlg, IDC_CHK_ASOUND, BST_CHECKED);
				CheckDlgButton(hwndDlg, IDC_RAD_SND1, BST_CHECKED);
				CheckDlgButton(hwndDlg, IDC_CHK_APOPUP, BST_CHECKED);

				hw = GetDlgItem(hwndDlg, IDC_ED_COMMAND);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_ED_PARAMS);
				EnableWindow(hw, FALSE);
			}
		}
		return TRUE;

	case WMU_UPDATETIME: 
		if (IsDlgButtonChecked(hwndDlg, IDC_RAD_ONCE))
			add_edit_alarm->occurrence = OC_ONCE;
		else if (IsDlgButtonChecked(hwndDlg, IDC_RAD_DAILY))
			add_edit_alarm->occurrence = OC_DAILY;
		else if (IsDlgButtonChecked(hwndDlg, IDC_RAD_WEEKDAYS))
			add_edit_alarm->occurrence = OC_WEEKDAYS;
		else if (IsDlgButtonChecked(hwndDlg, IDC_RAD_WEEKLY))
			add_edit_alarm->occurrence = OC_WEEKLY;
		else if (IsDlgButtonChecked(hwndDlg, IDC_RAD_MONTHLY))
			add_edit_alarm->occurrence = OC_MONTHLY;
		else if (IsDlgButtonChecked(hwndDlg, IDC_RAD_YEARLY)) {
			add_edit_alarm->occurrence = OC_YEARLY;
				
			// set date range to 1 year (now to 1 year in future)
			SYSTEMTIME r[2];
			GetPluginTime(&r[0]);
			TimeForMinutesInFuture(365 * 24 * 60, &r[1]);
			SendDlgItemMessage(hwndDlg, IDC_DATE, DTM_SETRANGE, (WPARAM)(GDTR_MIN | GDTR_MAX), (LPARAM)r);
		}

		if (add_edit_alarm->occurrence == OC_ONCE || add_edit_alarm->occurrence == OC_YEARLY)
			SendDlgItemMessage(hwndDlg, IDC_DATE, DTM_GETSYSTEMTIME, 0, (LPARAM)&add_edit_alarm->time);

		if (add_edit_alarm->occurrence == OC_WEEKLY)
			add_edit_alarm->time.wDayOfWeek = (unsigned short)SendDlgItemMessage(hwndDlg, IDC_DAY, CB_GETCURSEL, 0, 0);

		if (add_edit_alarm->occurrence == OC_MONTHLY)
			add_edit_alarm->time.wDay = (WORD)SendDlgItemMessage(hwndDlg, IDC_DAYNUM, CB_GETCURSEL, 0, 0) + 1;

		SYSTEMTIME temp_time;
		SendDlgItemMessage(hwndDlg, IDC_TIME, DTM_GETSYSTEMTIME, 0, (LPARAM)&temp_time);
		add_edit_alarm->time.wHour = temp_time.wHour;
		add_edit_alarm->time.wMinute = temp_time.wMinute;
		add_edit_alarm->time.wSecond = temp_time.wSecond;

		UpdateAlarm(add_edit_alarm->time, add_edit_alarm->occurrence);
		return TRUE;

	case WMU_SETTIME:
		{
			SYSTEMTIME temp_time;
			SendDlgItemMessage(hwndDlg, IDC_DATE, MCM_GETCURSEL, 0, (LPARAM)&temp_time);
			if (memcmp(&temp_time, &add_edit_alarm->time, sizeof(SYSTEMTIME)) != 0)
				SendDlgItemMessage(hwndDlg, IDC_DATE, MCM_SETCURSEL, 0, (LPARAM)&add_edit_alarm->time);

			SendDlgItemMessage(hwndDlg, IDC_DAY, CB_SETCURSEL, add_edit_alarm->time.wDayOfWeek, 0);
			SendDlgItemMessage(hwndDlg, IDC_TIME, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&add_edit_alarm->time);
			SendDlgItemMessage(hwndDlg, IDC_DAYNUM, CB_SETCURSEL, add_edit_alarm->time.wDay - 1, 0);
			SetDlgItemInt(hwndDlg, IDC_RELMIN, MinutesInFuture(add_edit_alarm->time, add_edit_alarm->occurrence), FALSE);
		}
		return TRUE;

	case WM_NOTIFY:
		if ( ((LPNMHDR)lParam)->code == DTN_DATETIMECHANGE) {
			switch( ((LPNMHDR)lParam)->idFrom ) {
			case IDC_TIME: 
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);

				SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);
				SendMessage(hwndDlg, WMU_SETTIME, 0, 0);
				break;

			case IDC_DATE: 
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);

				SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);
				SendMessage(hwndDlg, WMU_SETTIME, 0, 0);
				break;
			}
			return TRUE;
		}
		break;

	case WM_COMMAND:
		if ( HIWORD(wParam) == CBN_SELCHANGE ) {
			switch( LOWORD(wParam)) {
			case IDC_DAY: 
			case IDC_DAYNUM:
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);

				SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);
				SendMessage(hwndDlg, WMU_SETTIME, 0, 0);
				break;
			}
			return TRUE;
		}
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			switch( LOWORD( wParam )) {
			case IDC_RELMIN: 
				{
					BOOL translated;
					int mins;
					mins = GetDlgItemInt(hwndDlg, IDC_RELMIN, &translated, FALSE);
					SYSTEMTIME newtime;
					TimeForMinutesInFuture(mins, &newtime);

					SendDlgItemMessage(hwndDlg, IDC_TIME, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&newtime);
					SendDlgItemMessage(hwndDlg, IDC_DATE, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&newtime);
					SendDlgItemMessage(hwndDlg, IDC_DAY, CB_SETCURSEL, newtime.wDayOfWeek, 0);
					SendDlgItemMessage(hwndDlg, IDC_DAYNUM, CB_SETCURSEL, newtime.wDay - 1, 0);

					add_edit_alarm->time = newtime;
						
					SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);
				}
					// drop through
			case IDC_TITLE:
			case IDC_DESC:
			case IDC_ED_COMMAND:
			case IDC_ED_PARAMS:
				HWND hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);
			}
			return TRUE;
		}
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_RAD_ONCE:
				hw = GetDlgItem(hwndDlg, IDC_DAY);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_CAL);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_DATE);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_RELMIN);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
				EnableWindow(hw, FALSE);
				SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);
				SendMessage(hwndDlg, WMU_SETTIME, 0, 0);
				break;

			case IDC_RAD_DAILY:
				hw = GetDlgItem(hwndDlg, IDC_BTN_CAL);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_DAY);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_DATE);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_RELMIN);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
				EnableWindow(hw, TRUE);
				SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);
				SendMessage(hwndDlg, WMU_SETTIME, 0, 0);
				break;

			case IDC_RAD_WEEKLY:
				hw = GetDlgItem(hwndDlg, IDC_BTN_CAL);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_DATE);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_DAY);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_RELMIN);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
				EnableWindow(hw, TRUE);
				SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);
				SendMessage(hwndDlg, WMU_SETTIME, 0, 0);
				break;

			case IDC_RAD_WEEKDAYS:
				hw = GetDlgItem(hwndDlg, IDC_BTN_CAL);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_DATE);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_DAY);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_RELMIN);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
				EnableWindow(hw, TRUE);
				SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);
				SendMessage(hwndDlg, WMU_SETTIME, 0, 0);
				break;

			case IDC_RAD_MONTHLY:
				hw = GetDlgItem(hwndDlg, IDC_BTN_CAL);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_DATE);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_DAY);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_RELMIN);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
				EnableWindow(hw, TRUE);
				SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);
				SendMessage(hwndDlg, WMU_SETTIME, 0, 0);
				break;
				
			case IDC_RAD_YEARLY:
				hw = GetDlgItem(hwndDlg, IDC_DAY);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_CAL);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_DATE);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_DAYNUM);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_RELMIN);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_CHK_SUSPEND);
				EnableWindow(hw, TRUE);
				SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);
				SendMessage(hwndDlg, WMU_SETTIME, 0, 0);
				break;
				
			case IDC_CHK_ACOMMAND:
				if (IsDlgButtonChecked(hwndDlg, IDC_CHK_ACOMMAND)) {
					HWND hw = GetDlgItem(hwndDlg, IDC_ED_COMMAND);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_ED_PARAMS);
					EnableWindow(hw, TRUE);
				} else {
					HWND hw = GetDlgItem(hwndDlg, IDC_ED_COMMAND);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_ED_PARAMS);
					EnableWindow(hw, FALSE);
				}

			// drop through
			case IDC_CHK_ASOUND:
				if (IsDlgButtonChecked(hwndDlg, IDC_CHK_ASOUND)) {
					hw = GetDlgItem(hwndDlg, IDC_RAD_SND1);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_RAD_SND2);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_RAD_SND3);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_RAD_SPK);
					EnableWindow(hw, TRUE);
				} 
				else {
					hw = GetDlgItem(hwndDlg, IDC_RAD_SND1);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_RAD_SND2);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_RAD_SND3);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_RAD_SPK);
					EnableWindow(hw, FALSE);
				}

			// drop though
			case IDC_RAD_SND1:
			case IDC_RAD_SND2:
			case IDC_RAD_SND3:
			case IDC_RAD_SPK:
			case IDC_CHK_SUSPEND:
			case IDC_CHK_NOSTARTUP:
			case IDC_CHK_NOREMINDER:
			case IDC_CHK_APOPUP:
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);
				break;
				
			case IDOK: 
				{
					TCHAR buff[MAX_PATH];
					GetDlgItemText(hwndDlg, IDC_TITLE, buff, SIZEOF(buff));
					replaceStrT(add_edit_alarm->szTitle, buff);

					GetDlgItemText(hwndDlg, IDC_DESC, buff, SIZEOF(buff));
					replaceStrT(add_edit_alarm->szDesc, buff);

					if (add_edit_alarm->szTitle == 0 || add_edit_alarm->szTitle[0] == '\0') {
						MessageBox(hwndDlg, TranslateT("Please enter a title for this alarm."), TranslateT("Error"), MB_OK | MB_ICONERROR);
						return TRUE;
					}

					SendMessage(hwndDlg, WMU_UPDATETIME, 0, 0);

					if (!UpdateAlarm(add_edit_alarm->time, add_edit_alarm->occurrence)) {
						MessageBox(hwndDlg, TranslateT("The alarm time you have selected is in the past."), TranslateT("Error"), MB_OK | MB_ICONERROR);
						return TRUE;
					}

					add_edit_alarm->action = (IsDlgButtonChecked(hwndDlg, IDC_CHK_ASOUND) ? AAF_SOUND : 0);
					add_edit_alarm->action |= (IsDlgButtonChecked(hwndDlg, IDC_CHK_APOPUP) ? AAF_POPUP : 0);
					add_edit_alarm->action |= (IsDlgButtonChecked(hwndDlg, IDC_CHK_ACOMMAND) ? AAF_COMMAND : 0);

					add_edit_alarm->flags = add_edit_alarm->flags & ~(ALF_SUSPENDED | ALF_NOSTARTUP | ALF_NOREMINDER);
					if (add_edit_alarm->occurrence != OC_ONCE) {
						add_edit_alarm->flags |= IsDlgButtonChecked(hwndDlg, IDC_CHK_SUSPEND) ? ALF_SUSPENDED : 0;
					}
					add_edit_alarm->flags |= IsDlgButtonChecked(hwndDlg, IDC_CHK_NOSTARTUP) ? ALF_NOSTARTUP : 0;
					add_edit_alarm->flags |= IsDlgButtonChecked(hwndDlg, IDC_CHK_NOREMINDER) ? ALF_NOREMINDER: 0;

					if (add_edit_alarm->action & AAF_COMMAND) {
						GetDlgItemText(hwndDlg, IDC_ED_COMMAND, buff, SIZEOF(buff));
						replaceStrT(add_edit_alarm->szCommand, buff);
						GetDlgItemText(hwndDlg, IDC_ED_PARAMS, buff, SIZEOF(buff));
						replaceStrT(add_edit_alarm->szCommandParams, buff);
					}

					if (add_edit_alarm->action & AAF_SOUND) {
						if (IsDlgButtonChecked(hwndDlg, IDC_RAD_SND1))
							add_edit_alarm->sound_num = 1;
						else if (IsDlgButtonChecked(hwndDlg, IDC_RAD_SND2))
							add_edit_alarm->sound_num = 2;
						else if (IsDlgButtonChecked(hwndDlg, IDC_RAD_SND3))
							add_edit_alarm->sound_num = 3;
						else if (IsDlgButtonChecked(hwndDlg, IDC_RAD_SPK))
							add_edit_alarm->sound_num = 4;
					} else
						add_edit_alarm->sound_num = 0;


					Utils_SaveWindowPosition(hwndDlg, 0, MODULE, "AddEdit");

					// self-add (setting stored in invisible checkbox - see comments in WM_INITDIALOG
					if (IsDlgButtonChecked(hwndDlg, IDC_CHK_INVIS)) {
						alter_alarm_list(add_edit_alarm);
						RefreshReminderFrame();
						if (hwndOptionsDialog) {
							// refresh options list
							PostMessage(hwndOptionsDialog, WMU_INITOPTLIST, 0, 0);
						}
					}

					// self-add (setting stored in invisible checkbox - see comments in WM_INITDIALOG
					if (IsDlgButtonChecked(hwndDlg, IDC_CHK_INVIS)) {
						free_alarm_data(add_edit_alarm);
						delete add_edit_alarm;
					}

					// inform options dialog of change

					// deal with modal and non-modal modes
					PostMessage(hwndDlg, WMU_MYDESTROY, 0, 0);
					EndDialog(hwndDlg, IDOK);
				}
				break;
				
			case IDCANCEL:
				// self-add (setting stored in invisible checkbox - see comments in WM_INITDIALOG
				if (IsDlgButtonChecked(hwndDlg, IDC_CHK_INVIS)) {
					ALARM *add_edit_alarm = (ALARM *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					free_alarm_data(add_edit_alarm);
					delete add_edit_alarm;
				}
						
				Utils_SaveWindowPosition(hwndDlg, 0, MODULE, "AddEdit");
	
				// deal with modal and non-modal modes
				PostMessage(hwndDlg, WMU_MYDESTROY, 0, 0);
				EndDialog(hwndDlg, IDCANCEL);
				break;
		
			default:
				return TRUE;
			}
		}
		break;

	case WMU_MYDESTROY:
		DestroyWindow(hwndDlg);
		return TRUE;
	}

	return FALSE;
}

BOOL Edit(HWND hwnd, ALARM &alarm, bool modal)
{
	AddEditParam *param = new AddEditParam;
	param->alarm_ptr = &alarm;
	param->edit = TRUE;

	if (modal) {
		param->self_add = FALSE;
		if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hwnd, DlgProcAddEdit, (LPARAM)param) == IDOK) {
			return TRUE;
		}
	}
	else {
		param->self_add = TRUE;
		HWND hwndDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hwnd, DlgProcAddEdit, (LPARAM)param);

		//SetActiveWindow(hwndDlg);
		//SetWindowPos(hwndDlg, 
		SetForegroundWindow(hwndDlg);
		return TRUE;
	}
	return FALSE;
}

BOOL New(HWND hwnd, ALARM &alarm, bool modal)
{
	AddEditParam *param = new AddEditParam;
	param->alarm_ptr = &alarm;
	param->edit = FALSE;

	if (modal) {
		param->self_add = FALSE;
		if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hwnd, DlgProcAddEdit, (LPARAM)param) == IDOK)
			return TRUE;
	}
	else {
		param->self_add = TRUE;
		HWND hwndDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hwnd, DlgProcAddEdit, (LPARAM)param);

		//SetActiveWindow(hwndDlg);
		//SetWindowPos(hwndDlg, 
		SetForegroundWindow(hwndDlg);
		if (ServiceExists(MS_TTB_SETBUTTONSTATE)) {
			Sleep(100);
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTopToolbarButton, (LPARAM)0);
		}

		return TRUE;
	}
	return FALSE;
}

void AddMenuItem()
{
	if (hMainMenuItem) return;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ALL;
	mi.icolibItem = Skin_GetIconHandle("alarms_menu_set");
	mi.pszName = "Set alarm";
	mi.pszService = MODULE "/NewAlarm";
	mi.position = 500010000;
	if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		mi.pszPopupName = "Alarms";

	GroupMenuParam gmp = {0};
	Menu_AddGroupMenuItem(&gmp, &mi);

	hMainMenuItem = Menu_AddMainMenuItem(&mi);
}

///////////////////////
// create menu item

int OptionsModulesLoaded(WPARAM, LPARAM)
{
	CreateServiceFunction(MODULE "/NewAlarm", NewAlarmMenuFunc);

	AddMenuItem();

	return 0;
}

AlarmList temp_list, added_list, modified_list;		// we need to keep track of added and modified alarms, since if the non-modal dialog
													// refreshes the list before changes are applied, we loose them

class ShortList {
public:
	class Node {
	public:
		Node(): next(0) {}
		unsigned short value;
		Node *next;
	};


	ShortList(): head(0) {}
	virtual ~ShortList() {clear();}

	void clear() {
		Node *current;
		while(head) {
			current = head;
			head = head->next;
			delete current;
		}
	}

	void push_back(unsigned short s) {Node *n = new Node; n->value = s; n->next = head; head = n;}

	Node *get_head() {return head;}

protected:

	Node *head;
};

ShortList deleted_list;			// same with deleted items

Options temp_options;
static INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int sel;

	switch ( msg ) {
	case WM_INITDIALOG:
		hwndOptionsDialog = hwndDlg;
		added_list.clear();
		modified_list.clear();
		deleted_list.clear();

		TranslateDialogDefault( hwndDlg );

		temp_options = options;

		PostMessage(hwndDlg, WMU_INITOPTLIST, 0, 0);
		return TRUE;

	case WMU_INITOPTLIST:
		copy_list(temp_list);
		{
			// add added alarms, in case this is a list refresh from non-modal add alarm dialog
			ALARM *i, *j;
			for(added_list.reset(); i = added_list.current(); added_list.next()) {
				temp_list.push_back(i);
			}
			// modify modified alarms, in case this is a list refresh from non-modal add alarm dialog
			for(temp_list.reset(); i = temp_list.current(); temp_list.next()) {
				for(modified_list.reset(); j = modified_list.current(); modified_list.next()) {
					if (i->id == j->id) {
						copy_alarm_data(i, j);
					}
				}
			}
			// remove deleted alarms, in case this is a list refresh from non-modal add alarm dialog

			ShortList::Node *k = deleted_list.get_head();
			while(k) {
				for(temp_list.reset(); i = temp_list.current(); temp_list.next()) {
					if (i->id == k->value) {
						temp_list.erase();
						break;
					}
				}
				k = k->next;
			}

			SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_RESETCONTENT, 0, 0);

			if (temp_list.size() > 0) {
				int pos = -1;
				ALARM *i;
				for(temp_list.reset(); i = temp_list.current(); temp_list.next()) {
					if (!(i->flags & ALF_HIDDEN)) {
						pos = SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_ADDSTRING, 0, (LPARAM)i->szTitle);
						SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_SETITEMDATA, pos, (LPARAM)i->id);
					}
				}
				SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_SETCURSEL, pos, 0);
				HWND hw;
				hw = GetDlgItem(hwndDlg, IDC_EDIT);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_DEL);
				EnableWindow(hw, TRUE);
			} else {
				HWND hw;
				hw = GetDlgItem(hwndDlg, IDC_EDIT);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_DEL);
				EnableWindow(hw, FALSE);
			}

			if ( ServiceExists(MS_POPUP_ADDPOPUPT)) {
				CheckDlgButton(hwndDlg, IDC_CHK_POPUPS, temp_options.use_popup_module ? BST_CHECKED : BST_UNCHECKED);
				if (options.use_popup_module) {
					HWND hw = GetDlgItem(hwndDlg, IDC_CHK_LOOPSOUND);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_CHK_AWROUND);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_CHK_AWNOACTIVATE);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_SPIN_TRANS);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_ED_TRANS);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_BTN_PREVIEW);
					EnableWindow(hw, FALSE);
				} else {
					CheckDlgButton(hwndDlg, IDC_CHK_LOOPSOUND, temp_options.loop_sound ? BST_CHECKED : BST_UNCHECKED);
					HWND hw = GetDlgItem(hwndDlg, IDC_CHK_LOOPSOUND);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_CHK_AWROUND);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_CHK_AWNOACTIVATE);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_SPIN_TRANS);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_ED_TRANS);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_BTN_PREVIEW);
					EnableWindow(hw, TRUE);
				}
			} else {
				HWND hw = GetDlgItem(hwndDlg, IDC_CHK_POPUPS);
				EnableWindow(hw, FALSE);
				CheckDlgButton(hwndDlg, IDC_CHK_LOOPSOUND, temp_options.loop_sound ? BST_CHECKED : BST_UNCHECKED);
			
				hw = GetDlgItem(hwndDlg, IDC_CHK_LOOPSOUND);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_CHK_AWROUND);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_CHK_AWNOACTIVATE);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_SPIN_TRANS);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_ED_TRANS);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_PREVIEW);
				EnableWindow(hw, TRUE);
			}

			SendDlgItemMessage(hwndDlg, IDC_SPIN_SNOOZE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(360, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_SNOOZE, UDM_SETPOS, 0, temp_options.snooze_minutes);

			SendDlgItemMessage(hwndDlg, IDC_SPIN_ROWHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 5));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_ROWHEIGHT, UDM_SETPOS, 0, temp_options.row_height);

			SendDlgItemMessage(hwndDlg, IDC_SPIN_INDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_INDENT, UDM_SETPOS, 0, temp_options.indent);

			SendDlgItemMessage(hwndDlg, IDC_SPIN_TRANS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_TRANS, UDM_SETPOS, 0, temp_options.aw_trans);
		}

		CheckDlgButton(hwndDlg, IDC_SHOWHIDE, temp_options.auto_showhide ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWHIDE2, temp_options.hide_with_clist ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOSIZEVERT, temp_options.auto_size_vert ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_CHK_AWROUND, temp_options.aw_roundcorners ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_AWNOACTIVATE, temp_options.aw_dontstealfocus ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_SPIN_PERIOD, UDM_SETRANGE, 0, (LPARAM)MAKELONG(72, 1));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_PERIOD, UDM_SETPOS, 0, temp_options.reminder_period);

		SendMessage( GetParent( hwndDlg ), PSM_UNCHANGED, 0, 0 ); 
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus())
			SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );

		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_BTN_PREVIEW:
				{
					HWND hwndDlgPrev = CreateDialog(hInst, MAKEINTRESOURCE(IDD_ALARM), GetDesktopWindow(), DlgProcAlarm);
					WindowList_Add(hAlarmWindowList, hwndDlgPrev, 0);
					SendMessage(hwndDlgPrev, WMU_FAKEALARM, 0, 0);
					SendMessage(hwndDlgPrev, WMU_SETOPT, 0, (LPARAM)&temp_options);
					ShowWindow(hwndDlgPrev, SW_SHOW);
					SetWindowPos(hwndDlgPrev, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				}
				break;
			case IDC_SHOWHIDE:
				temp_options.auto_showhide = IsDlgButtonChecked(hwndDlg, IDC_SHOWHIDE) ? true : false;
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			case IDC_SHOWHIDE2:
				temp_options.hide_with_clist = IsDlgButtonChecked(hwndDlg, IDC_SHOWHIDE2) ? true : false;
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			case IDC_AUTOSIZEVERT:
				temp_options.auto_size_vert = IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZEVERT) ? true : false;
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			case IDC_NEW: 
				{
					ALARM new_alarm = {0};
					GetPluginTime(&new_alarm.time);
					new_alarm.id = next_alarm_id++;
					if (New(hwndDlg, new_alarm, true)) {
						new_alarm.id = next_alarm_id++;
						temp_list.push_back(&new_alarm);
						added_list.push_back(&new_alarm);
						int pos = SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_ADDSTRING, (WPARAM)-1, (LPARAM)new_alarm.szTitle);
						SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_SETITEMDATA, pos, new_alarm.id);
						SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_SETCURSEL, pos, 0);
						HWND hw;
						hw = GetDlgItem(hwndDlg, IDC_EDIT);
						EnableWindow(hw, TRUE);
						hw = GetDlgItem(hwndDlg, IDC_DEL);
						EnableWindow(hw, TRUE);
						SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );

						free_alarm_data(&new_alarm);
					}
				}
				break;

			case IDC_EDIT: 
				sel = SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_GETCURSEL, 0, 0);
				if (sel != -1) {
					unsigned short id = (unsigned short)SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_GETITEMDATA, sel, 0);
					ALARM *i;
					for(temp_list.reset(); i = temp_list.current(); temp_list.next()) {
						if (i->id == id) {
							ALARM a = {0};
							copy_alarm_data(&a, i);
							if (Edit(hwndDlg, a, true)) {
								modified_list.push_back(&a);
								SendMessage(hwndDlg, WMU_INITOPTLIST, 0, 0);
								SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_SETCURSEL, sel, 0);
								SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
							}
							free_alarm_data(&a);
							break;
						}
					}
				}
				break;

			case IDC_DEL:
				sel = SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_GETCURSEL, 0, 0);
				if (sel != -1) {
					unsigned short id = (unsigned short)SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_GETITEMDATA, sel, 0);

					ALARM *i;
					for(temp_list.reset(); i = temp_list.current(); temp_list.next()) {
						if (i->id == id) {
							deleted_list.push_back(i->id);
							//temp_list.erase();
							break;
						}
					}
							
					SendMessage( hwndDlg, WMU_INITOPTLIST, 0, 0 );
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}
				break;
				
			case IDC_CHK_POPUPS: 
				{
					BOOL chk = IsDlgButtonChecked(hwndDlg, IDC_CHK_POPUPS);
					if (chk) CheckDlgButton(hwndDlg, IDC_CHK_LOOPSOUND, BST_UNCHECKED);
					else CheckDlgButton(hwndDlg, IDC_CHK_LOOPSOUND, temp_options.loop_sound ? BST_CHECKED : BST_UNCHECKED);

					HWND hw = GetDlgItem(hwndDlg, IDC_CHK_LOOPSOUND);
					EnableWindow(hw, !chk);
					hw = GetDlgItem(hwndDlg, IDC_CHK_AWROUND);
					EnableWindow(hw, !chk);
					hw = GetDlgItem(hwndDlg, IDC_CHK_AWNOACTIVATE);
					EnableWindow(hw, !chk);
					hw = GetDlgItem(hwndDlg, IDC_SPIN_TRANS);
					EnableWindow(hw, !chk);
					hw = GetDlgItem(hwndDlg, IDC_ED_TRANS);
					EnableWindow(hw, !chk);
					hw = GetDlgItem(hwndDlg, IDC_BTN_PREVIEW);
					EnableWindow(hw, !chk);

					temp_options.use_popup_module = IsDlgButtonChecked(hwndDlg, IDC_CHK_POPUPS) ? true : false;
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 ); 
				}
				break;
				
			case IDC_CHK_LOOPSOUND:
				temp_options.loop_sound = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOOPSOUND) ? true : false;
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 ); 
				break;

			case IDC_CHK_AWROUND:
				temp_options.aw_roundcorners = IsDlgButtonChecked(hwndDlg, IDC_CHK_AWROUND) ? true : false;
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 ); 										
				break;

			case IDC_CHK_AWNOACTIVATE:
				temp_options.aw_dontstealfocus = IsDlgButtonChecked(hwndDlg, IDC_CHK_AWNOACTIVATE) ? true : false;
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 ); 										
				break;
			}
			return TRUE;
		}

		if (HIWORD( wParam ) == LBN_SELCHANGE) {
			HWND hw;
			hw = GetDlgItem(hwndDlg, IDC_EDIT);
			EnableWindow(hw, TRUE);
			hw = GetDlgItem(hwndDlg, IDC_DEL);
			EnableWindow(hw, TRUE);

			//SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}

		if (HIWORD( wParam ) == LBN_DBLCLK) {
			int sel = SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_GETCURSEL, 0, 0);
			if (sel != -1) {
				unsigned short id = (unsigned short)SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_GETITEMDATA, sel, 0);
				ALARM *i;
				for(temp_list.reset(); i = temp_list.current(); temp_list.next()) {
					if (i->id == id) {
						ALARM a = {0};
						copy_alarm_data(&a, i);
						if (Edit(hwndDlg, a, true)) {
							modified_list.push_back(&a);
							SendMessage(hwndDlg, WMU_INITOPTLIST, 0, 0);
							SendDlgItemMessage(hwndDlg, IDC_ALIST, LB_SETCURSEL, sel, 0);
							SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
						}
						free_alarm_data(&a);
						break;
					}
				}
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == UDN_DELTAPOS ) {
			BOOL translated;
			int trans = GetDlgItemInt(hwndDlg, IDC_ED_TRANS, &translated, FALSE);
			if (translated) temp_options.aw_trans = trans;
			SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
			return TRUE;
		}
		if ((int)((LPNMHDR)lParam)->code == PSN_APPLY ) {
			set_list(temp_list);
			SaveAlarms();

			BOOL translated;
			int snooze_mins = GetDlgItemInt(hwndDlg, IDC_ED_SNOOZE, &translated, FALSE);
			if (translated) temp_options.snooze_minutes = snooze_mins;
			int row_height = GetDlgItemInt(hwndDlg, IDC_ED_ROWHEIGHT, &translated, FALSE);
			if (translated) temp_options.row_height = row_height;
			int indent = GetDlgItemInt(hwndDlg, IDC_ED_INDENT, &translated, FALSE);
			if (translated) temp_options.indent = indent;
			int trans = GetDlgItemInt(hwndDlg, IDC_ED_TRANS, &translated, FALSE);
			if (translated) temp_options.aw_trans = trans;

			temp_options.use_popup_module = IsDlgButtonChecked(hwndDlg, IDC_CHK_POPUPS) ? true : false;
			temp_options.auto_showhide = IsDlgButtonChecked(hwndDlg, IDC_SHOWHIDE) ? true : false;
			temp_options.hide_with_clist = IsDlgButtonChecked(hwndDlg, IDC_SHOWHIDE2) ? true : false;
			temp_options.auto_size_vert = IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZEVERT) ? true : false;
			temp_options.aw_roundcorners = IsDlgButtonChecked(hwndDlg, IDC_CHK_AWROUND) ? true : false;
			temp_options.aw_dontstealfocus = IsDlgButtonChecked(hwndDlg, IDC_CHK_AWNOACTIVATE) ? true : false;

			int reminder_period = GetDlgItemInt(hwndDlg, IDC_ED_PERIOD, &translated, FALSE);
			if (translated) temp_options.reminder_period = reminder_period;

			if ( !ServiceExists(MS_POPUP_ADDPOPUPT) || !options.use_popup_module)
				temp_options.loop_sound = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOOPSOUND) ? true : false;

			options = temp_options;
			SaveOptions();
			FixMainMenu();
			RefreshReminderFrame();
			SetAlarmWinOptions();

			added_list.clear();
			modified_list.clear();
			deleted_list.clear();
			return TRUE;
		}

		break;

	case WM_DESTROY:
		hwndOptionsDialog = 0;
		added_list.clear();
		modified_list.clear();
		deleted_list.clear();
		break;
	}

	return FALSE;
}


int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = -790000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT1);
	odp.pszTitle = LPGEN("Alarms");
	odp.pszGroup = LPGEN("Events");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}

void LoadOptions()
{
	options.use_popup_module = (db_get_b(0, MODULE, "UsePopupModule", 0) == 1);
	options.snooze_minutes = (int)db_get_dw(0, MODULE, "SnoozeMinutes", 10);
	options.row_height = (int)db_get_dw(0, MODULE, "RowHeight", 20);
	options.indent = (int)db_get_dw(0, MODULE, "Indent", 5);
	options.aw_trans = (int)db_get_b(0, MODULE, "Transparency", 0);
	options.aw_roundcorners = (db_get_b(0, MODULE, "RoundCorners", 1) == 1);
	options.aw_dontstealfocus = (db_get_b(0, MODULE, "DontStealFocus", 1) == 1);
	options.auto_showhide = (db_get_b(0, MODULE, "AutoShowHide", 0) == 1);
	options.hide_with_clist = (db_get_b(0, MODULE, "HideWithClist", 0) == 1);
	options.loop_sound = (db_get_b(0, MODULE, "LoopSound", 1) == 1);
	options.auto_size_vert = (db_get_b(0, MODULE, "AutoSize", 0) == 1);
	options.reminder_period = (int)db_get_dw(0, MODULE, "ReminderPeriod", 8);

	HookEvent(ME_SYSTEM_MODULESLOADED, OptionsModulesLoaded);
}

void SaveOptions()
{
	db_set_b(0, MODULE, "UsePopupModule", options.use_popup_module ? 1 : 0);
	db_set_dw(0, MODULE, "SnoozeMinutes", options.snooze_minutes);
	db_set_dw(0, MODULE, "RowHeight", options.row_height);
	db_set_dw(0, MODULE, "Indent", options.indent);
	db_set_b(0, MODULE, "Transparency", options.aw_trans);
	db_set_b(0, MODULE, "RoundCorners", options.aw_roundcorners ? 1 : 0);
	db_set_b(0, MODULE, "DontStealFocus", options.aw_dontstealfocus ? 1 : 0);
	db_set_b(0, MODULE, "AutoShowHide", options.auto_showhide ? 1 : 0);
	db_set_b(0, MODULE, "HideWithClist", options.hide_with_clist ? 1 : 0);
	db_set_b(0, MODULE, "LoopSound", options.loop_sound ? 1 : 0);
	db_set_b(0, MODULE, "AutoSize", options.auto_size_vert ? 1 : 0);
	db_set_dw(0, MODULE, "ReminderPeriod", options.reminder_period);
}

INT_PTR NewAlarmMenuFunc(WPARAM, LPARAM)
{
	ALARM *new_alarm = new ALARM;
	memset(new_alarm, 0, sizeof(ALARM));
	new_alarm->id = next_alarm_id++;
	GetPluginTime(&new_alarm->time);

	//New((HWND)CallService(MS_CLUI_GETHWND, 0, 0), *new_alarm, false);
	New(GetDesktopWindow(), *new_alarm, false);

	return 0;
}

void EditNonModal(ALARM &alarm)
{
	ALARM *new_alarm = new ALARM;
	memset(new_alarm, 0, sizeof(ALARM));
	copy_alarm_data(new_alarm, &alarm);

	Edit(GetDesktopWindow(), *new_alarm, false);
}
