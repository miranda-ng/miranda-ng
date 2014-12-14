/*

'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"

/* Menu Item */
static HANDLE hServiceMenuCommand;
/* Services */
static HANDLE hServiceShowDlg;
static HWND hwndSettingsDlg;
extern HINSTANCE hInst;

const DWORD unitValues[]={1,60,60*60,60*60*24,60*60*24*7,60*60*24*31};
const TCHAR *unitNames[]={LPGENT("Second(s)"), LPGENT("Minute(s)"), LPGENT("Hour(s)"), LPGENT("Day(s)"), LPGENT("Week(s)"), LPGENT("Month(s)")};

/************************* Dialog *************************************/

static void EnableDlgItem(HWND hwndDlg,int idCtrl,BOOL fEnable)
{
	hwndDlg=GetDlgItem(hwndDlg,idCtrl);
	if (hwndDlg != NULL && IsWindowEnabled(hwndDlg) != fEnable)
		EnableWindow(hwndDlg,fEnable);
}

static BOOL CALLBACK DisplayCpuUsageProc(BYTE nCpuUsage,LPARAM lParam)
{
	/* dialog closed? */
	if (!IsWindow((HWND)lParam)) return FALSE; /* stop poll thread */
	TCHAR str[64];
	mir_sntprintf(str,SIZEOF(str),TranslateT("(current: %u%%)"),nCpuUsage);
	SetWindowText((HWND)lParam,str);
	return TRUE;
}

static bool AnyProtoHasCaps(DWORD caps1)
{
	int nProtoCount;
	PROTOACCOUNT **protos;
	if (!ProtoEnumAccounts(&nProtoCount, &protos))
		for(int i=0;i<nProtoCount;++i)
			if (CallProtoService(protos[i]->szModuleName,PS_GETCAPS,(WPARAM)PFLAGNUM_1,0)&caps1)
				return true; /* CALLSERVICE_NOTFOUND also handled gracefully */
	return false;
}

#define M_ENABLE_SUBCTLS       (WM_APP+111)
#define M_UPDATE_SHUTDOWNDESC  (WM_APP+112)
#define M_CHECK_DATETIME       (WM_APP+113)
static INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			LCID locale;
			hwndSettingsDlg=hwndDlg;
			TranslateDialogDefault(hwndDlg);
			locale=CallService(MS_LANGPACK_GETLOCALE,0,0);
			SendDlgItemMessage(hwndDlg,IDC_ICON_HEADER,STM_SETIMAGE,IMAGE_ICON,(LPARAM)Skin_GetIcon("AutoShutdown_Header"));
			{
				HFONT hBoldFont;
				LOGFONT lf;
				if (GetObject((HFONT)SendDlgItemMessage(hwndDlg,IDC_TEXT_HEADER,WM_GETFONT,0,0),sizeof(lf),&lf)) {
					lf.lfWeight=FW_BOLD;
					hBoldFont=CreateFontIndirect(&lf);
				} else hBoldFont=NULL;
				SendDlgItemMessage(hwndDlg,IDC_TEXT_HEADER,WM_SETFONT,(WPARAM)hBoldFont,FALSE);
			}
			/* read-in watcher flags */
			{
				WORD watcherType=db_get_w(NULL,"AutoShutdown","WatcherFlags",SETTING_WATCHERFLAGS_DEFAULT);
				CheckRadioButton(hwndDlg,IDC_RADIO_STTIME,IDC_RADIO_STCOUNTDOWN,(watcherType&SDWTF_ST_TIME)?IDC_RADIO_STTIME:IDC_RADIO_STCOUNTDOWN);
				CheckDlgButton(hwndDlg,IDC_CHECK_SPECIFICTIME,(watcherType&SDWTF_SPECIFICTIME) != 0 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg,IDC_CHECK_MESSAGE,(watcherType&SDWTF_MESSAGE) != 0 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg,IDC_CHECK_FILETRANSFER,(watcherType&SDWTF_FILETRANSFER) != 0 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg,IDC_CHECK_IDLE,(watcherType&SDWTF_IDLE) != 0 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg,IDC_CHECK_STATUS,(watcherType&SDWTF_STATUS) != 0 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg,IDC_CHECK_CPUUSAGE,(watcherType&SDWTF_CPUUSAGE) != 0 ? BST_CHECKED : BST_UNCHECKED);
			}
			/* read-in countdown val */
			{
				SYSTEMTIME st;
				if (!TimeStampToSystemTime((time_t)db_get_dw(NULL,"AutoShutdown","TimeStamp",SETTING_TIMESTAMP_DEFAULT),&st))
					GetLocalTime(&st);
				DateTime_SetSystemtime(GetDlgItem(hwndDlg,IDC_TIME_TIMESTAMP),GDT_VALID,&st);
				DateTime_SetSystemtime(GetDlgItem(hwndDlg,IDC_DATE_TIMESTAMP),GDT_VALID,&st);
				SendMessage(hwndDlg,M_CHECK_DATETIME,0,0);
			}
			{
				DWORD setting=db_get_dw(NULL,"AutoShutdown","Countdown",SETTING_COUNTDOWN_DEFAULT);
				if (setting<1) setting=SETTING_COUNTDOWN_DEFAULT;
				SendDlgItemMessage(hwndDlg,IDC_SPIN_COUNTDOWN,UDM_SETRANGE,0,MAKELPARAM(UD_MAXVAL,1));
				SendDlgItemMessage(hwndDlg,IDC_EDIT_COUNTDOWN,EM_SETLIMITTEXT,(WPARAM)10,0);
				SendDlgItemMessage(hwndDlg,IDC_SPIN_COUNTDOWN,UDM_SETPOS,0,MAKELPARAM(setting,0));
				SetDlgItemInt(hwndDlg,IDC_EDIT_COUNTDOWN,setting,FALSE);
			}
			{
				HWND hwndCombo=GetDlgItem(hwndDlg,IDC_COMBO_COUNTDOWNUNIT);
				DWORD lastUnit=db_get_dw(NULL,"AutoShutdown","CountdownUnit",SETTING_COUNTDOWNUNIT_DEFAULT);
				SendMessage(hwndCombo,CB_SETLOCALE,(WPARAM)locale,0); /* sort order */
				SendMessage(hwndCombo,CB_INITSTORAGE,SIZEOF(unitNames),SIZEOF(unitNames)*16); /* approx. */
				for(int i=0;i<SIZEOF(unitNames);++i) {
					int index=SendMessage(hwndCombo,CB_ADDSTRING,0,(LPARAM)TranslateTS(unitNames[i]));
					if (index != LB_ERR) {
						SendMessage(hwndCombo,CB_SETITEMDATA,index,(LPARAM)unitValues[i]);
						if (i==0 || unitValues[i]==lastUnit) SendMessage(hwndCombo,CB_SETCURSEL,index,0);
					}
				}
			}
			{
				DBVARIANT dbv;
				if (!db_get_ts(NULL,"AutoShutdown","Message",&dbv)) {
					SetDlgItemText(hwndDlg,IDC_EDIT_MESSAGE,dbv.ptszVal);
					mir_free(dbv.ptszVal);
				}
			}
			/* cpuusage threshold */
			{
				BYTE setting=DBGetContactSettingRangedByte(NULL,"AutoShutdown","CpuUsageThreshold",SETTING_CPUUSAGETHRESHOLD_DEFAULT,1,100);
				SendDlgItemMessage(hwndDlg,IDC_SPIN_CPUUSAGE,UDM_SETRANGE,0,MAKELPARAM(100,1));
				SendDlgItemMessage(hwndDlg,IDC_EDIT_CPUUSAGE,EM_SETLIMITTEXT,(WPARAM)3,0);
				SendDlgItemMessage(hwndDlg,IDC_SPIN_CPUUSAGE,UDM_SETPOS,0,MAKELPARAM(setting,0));
				SetDlgItemInt(hwndDlg,IDC_EDIT_CPUUSAGE,setting,FALSE);
			}
			/* shutdown types */
			{
				HWND hwndCombo=GetDlgItem(hwndDlg,IDC_COMBO_SHUTDOWNTYPE);
				BYTE lastShutdownType=db_get_b(NULL,"AutoShutdown","ShutdownType",SETTING_SHUTDOWNTYPE_DEFAULT);
				SendMessage(hwndCombo,CB_SETLOCALE,(WPARAM)locale,0); /* sort order */
				SendMessage(hwndCombo,CB_SETEXTENDEDUI,TRUE,0);
				SendMessage(hwndCombo,CB_INITSTORAGE,SDSDT_MAX,SDSDT_MAX*32);
				for(BYTE shutdownType=1;shutdownType<=SDSDT_MAX;++shutdownType)
					if (ServiceIsTypeEnabled(shutdownType,0)) {
						TCHAR *pszText=(TCHAR*)ServiceGetTypeDescription(shutdownType,GSTDF_TCHAR); /* never fails */
						int index=SendMessage(hwndCombo,CB_ADDSTRING,0,(LPARAM)pszText);
						if (index != LB_ERR) {
							SendMessage(hwndCombo,CB_SETITEMDATA,index,(LPARAM)shutdownType);
							if (shutdownType==1 || shutdownType==lastShutdownType) SendMessage(hwndCombo,CB_SETCURSEL,(WPARAM)index,0);
						}
					}
				SendMessage(hwndDlg,M_UPDATE_SHUTDOWNDESC,0,(LPARAM)hwndCombo);
			}
			/* check if proto is installed that supports instant messages and check if a message dialog plugin is installed */
			if (!AnyProtoHasCaps(PF1_IMRECV) || !ServiceExists(MS_MSG_SENDMESSAGE)) { /* no srmessage present? */
				CheckDlgButton(hwndDlg, IDC_CHECK_MESSAGE, BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_MESSAGE),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_MESSAGE),FALSE);
			}
			/* check if proto is installed that supports file transfers and check if a file transfer dialog is available */
			if (!AnyProtoHasCaps(PF1_FILESEND) && !AnyProtoHasCaps(PF1_FILERECV)) {  /* no srfile present? */
				CheckDlgButton(hwndDlg, IDC_CHECK_FILETRANSFER, BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_FILETRANSFER),FALSE);
			}
			/* check if cpu usage can be detected */
			if (!PollCpuUsage(DisplayCpuUsageProc,(LPARAM)GetDlgItem(hwndDlg,IDC_TEXT_CURRENTCPU),1800)) {
				CheckDlgButton(hwndDlg, IDC_CHECK_CPUUSAGE, BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_CPUUSAGE),FALSE);
			}
			SendMessage(hwndDlg,M_ENABLE_SUBCTLS,0,0);
			Utils_RestoreWindowPositionNoSize(hwndDlg,NULL,"AutoShutdown","SettingsDlg_");
			return TRUE; /* default focus */
		}
	case WM_DESTROY:
		{
			Utils_SaveWindowPosition(hwndDlg,NULL,"AutoShutdown","SettingsDlg_");
			HICON hIcon=(HICON)SendDlgItemMessage(hwndDlg,IDC_ICON_HEADER,STM_SETIMAGE,IMAGE_ICON,0);
			HFONT hFont=(HFONT)SendDlgItemMessage(hwndDlg,IDC_TEXT_HEADER,WM_GETFONT,0,0);
			SendDlgItemMessage(hwndDlg,IDC_TEXT_HEADER,WM_SETFONT,0,FALSE); /* no return value */
			if (hFont != NULL) DeleteObject(hFont);
			hwndSettingsDlg=NULL;
		}
		return TRUE;

	case WM_CTLCOLORSTATIC:
		switch(GetDlgCtrlID((HWND)lParam)) {
		case IDC_ICON_HEADER:
			SetBkMode((HDC)wParam,TRANSPARENT);
		case IDC_RECT_HEADER:
			/* need to set COLOR_WINDOW manually for Win9x */
			SetBkColor((HDC)wParam,GetSysColor(COLOR_WINDOW));
			return (BOOL)GetSysColorBrush(COLOR_WINDOW);
		case IDC_TEXT_HEADER:
		case IDC_TEXT_HEADERDESC:
			SetBkMode((HDC)wParam,TRANSPARENT);
			return (BOOL)GetStockObject(NULL_BRUSH);
		}
		break;

	case M_ENABLE_SUBCTLS:
		{
			BOOL checked=IsDlgButtonChecked(hwndDlg,IDC_CHECK_MESSAGE) != 0;
			EnableDlgItem(hwndDlg,IDC_EDIT_MESSAGE,checked);
			checked=IsDlgButtonChecked(hwndDlg,IDC_CHECK_SPECIFICTIME) != 0;
			EnableDlgItem(hwndDlg,IDC_RADIO_STTIME,checked);
			EnableDlgItem(hwndDlg,IDC_RADIO_STCOUNTDOWN,checked);
			checked=(IsDlgButtonChecked(hwndDlg,IDC_CHECK_SPECIFICTIME) && IsDlgButtonChecked(hwndDlg,IDC_RADIO_STTIME));
			EnableDlgItem(hwndDlg,IDC_TIME_TIMESTAMP,checked);
			EnableDlgItem(hwndDlg,IDC_DATE_TIMESTAMP,checked);
			checked=(IsDlgButtonChecked(hwndDlg,IDC_CHECK_SPECIFICTIME) && IsDlgButtonChecked(hwndDlg,IDC_RADIO_STCOUNTDOWN));
			EnableDlgItem(hwndDlg,IDC_EDIT_COUNTDOWN,checked);
			EnableDlgItem(hwndDlg,IDC_SPIN_COUNTDOWN,checked);
			EnableDlgItem(hwndDlg,IDC_COMBO_COUNTDOWNUNIT,checked);
			checked=IsDlgButtonChecked(hwndDlg,IDC_CHECK_IDLE) != 0;
			EnableDlgItem(hwndDlg,IDC_URL_IDLE,checked);
			checked=IsDlgButtonChecked(hwndDlg,IDC_CHECK_CPUUSAGE) != 0;
			EnableDlgItem(hwndDlg,IDC_EDIT_CPUUSAGE,checked);
			EnableDlgItem(hwndDlg,IDC_SPIN_CPUUSAGE,checked);
			EnableDlgItem(hwndDlg,IDC_TEXT_PERCENT,checked);
			EnableDlgItem(hwndDlg,IDC_TEXT_CURRENTCPU,checked);
			checked=(IsDlgButtonChecked(hwndDlg,IDC_CHECK_SPECIFICTIME) || IsDlgButtonChecked(hwndDlg,IDC_CHECK_MESSAGE) ||
				IsDlgButtonChecked(hwndDlg,IDC_CHECK_IDLE) || IsDlgButtonChecked(hwndDlg,IDC_CHECK_STATUS) ||
				IsDlgButtonChecked(hwndDlg,IDC_CHECK_FILETRANSFER) || IsDlgButtonChecked(hwndDlg,IDC_CHECK_CPUUSAGE));
			EnableDlgItem(hwndDlg,IDOK,checked);
		}
		return TRUE;
	case M_UPDATE_SHUTDOWNDESC: /* lParam=(LPARAM)(HWND)hwndCombo */
		{
			BYTE shutdownType = (BYTE)SendMessage((HWND)lParam,CB_GETITEMDATA,SendMessage((HWND)lParam,CB_GETCURSEL,0,0),0);
			SetDlgItemText(hwndDlg,IDC_TEXT_SHUTDOWNTYPE,(TCHAR*)ServiceGetTypeDescription(shutdownType,GSTDF_LONGDESC|GSTDF_TCHAR));
		}
		return TRUE;

	case WM_TIMECHANGE: /* system time changed */
		SendMessage(hwndDlg,M_CHECK_DATETIME,0,0);
		return TRUE;

	case M_CHECK_DATETIME:
		{
			SYSTEMTIME st,stBuf;
			time_t timestamp;
			DateTime_GetSystemtime(GetDlgItem(hwndDlg,IDC_DATE_TIMESTAMP),&stBuf);
			DateTime_GetSystemtime(GetDlgItem(hwndDlg,IDC_TIME_TIMESTAMP),&st);
			st.wDay=stBuf.wDay;
			st.wDayOfWeek=stBuf.wDayOfWeek;
			st.wMonth=stBuf.wMonth;
			st.wYear=stBuf.wYear;
			GetLocalTime(&stBuf);
			if (SystemTimeToTimeStamp(&st,&timestamp)) {
				/* set to current date if earlier */
				if (timestamp<time(NULL)) {
					st.wDay=stBuf.wDay;
					st.wDayOfWeek=stBuf.wDayOfWeek;
					st.wMonth=stBuf.wMonth;
					st.wYear=stBuf.wYear;
					if (SystemTimeToTimeStamp(&st,&timestamp)) {
						/* step one day up if still earlier */
						if (timestamp<time(NULL)) {
							timestamp+=24*60*60;
							TimeStampToSystemTime(timestamp,&st);
						}
					}
				}
			}
			DateTime_SetRange(GetDlgItem(hwndDlg,IDC_DATE_TIMESTAMP),GDTR_MIN,&stBuf);
			DateTime_SetRange(GetDlgItem(hwndDlg,IDC_TIME_TIMESTAMP),GDTR_MIN,&stBuf);
			DateTime_SetSystemtime(GetDlgItem(hwndDlg,IDC_DATE_TIMESTAMP),GDT_VALID,&st);
			DateTime_SetSystemtime(GetDlgItem(hwndDlg,IDC_TIME_TIMESTAMP),GDT_VALID,&st);
			return TRUE;
		}
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->idFrom) {
		case IDC_TIME_TIMESTAMP:
		case IDC_DATE_TIMESTAMP:
			switch(((NMHDR*)lParam)->code) {
			case DTN_CLOSEUP:
			case NM_KILLFOCUS:
				PostMessage(hwndDlg,M_CHECK_DATETIME,0,0);
				return TRUE;
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_CHECK_MESSAGE:
		case IDC_CHECK_FILETRANSFER:
		case IDC_CHECK_IDLE:
		case IDC_CHECK_CPUUSAGE:
		case IDC_CHECK_STATUS:
		case IDC_CHECK_SPECIFICTIME:
		case IDC_RADIO_STTIME:
		case IDC_RADIO_STCOUNTDOWN:
			SendMessage(hwndDlg,M_ENABLE_SUBCTLS,0,0);
			return TRUE;

		case IDC_EDIT_COUNTDOWN:
			if (HIWORD(wParam)==EN_KILLFOCUS) {
				if ((int)GetDlgItemInt(hwndDlg,IDC_EDIT_COUNTDOWN,NULL,TRUE)<1) {
					SendDlgItemMessage(hwndDlg,IDC_SPIN_COUNTDOWN,UDM_SETPOS,0,MAKELPARAM(1,0));
					SetDlgItemInt(hwndDlg,IDC_EDIT_COUNTDOWN,1,FALSE);
				}
				return TRUE;
			}
			break;

		case IDC_EDIT_CPUUSAGE:
			if (HIWORD(wParam)==EN_KILLFOCUS) {
				WORD val=(WORD)GetDlgItemInt(hwndDlg,IDC_EDIT_CPUUSAGE,NULL,FALSE);
				if (val<1) val=1;
				else if (val>100) val=100;
				SendDlgItemMessage(hwndDlg,IDC_SPIN_CPUUSAGE,UDM_SETPOS,0,MAKELPARAM(val,0));
				SetDlgItemInt(hwndDlg,IDC_EDIT_CPUUSAGE,val,FALSE);
				return TRUE;
			}
			break;

		case IDC_URL_IDLE:
			{	
				OPENOPTIONSDIALOG ood;
				ood.cbSize = sizeof(ood);
				ood.pszGroup = "Status"; /* autotranslated */
				ood.pszPage = "Idle"; /* autotranslated */
				ood.pszTab = NULL;
				Options_Open(&ood);
				return TRUE;
			}

		case IDC_COMBO_SHUTDOWNTYPE:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				SendMessage(hwndDlg,M_UPDATE_SHUTDOWNDESC,0,lParam);
			return TRUE;

		case IDOK: /* save settings and start watcher */
			ShowWindow(hwndDlg,SW_HIDE);
			/* message text */
			{
				HWND hwndEdit=GetDlgItem(hwndDlg,IDC_EDIT_MESSAGE);
				int len=GetWindowTextLength(hwndEdit)+1;
				TCHAR *pszText=(TCHAR*)mir_alloc(len*sizeof(TCHAR));
				if (pszText != NULL && GetWindowText(hwndEdit,pszText,len+1)) {
					TrimString(pszText);
					db_set_ts(NULL,"AutoShutdown","Message",pszText);
				}
				mir_free(pszText); /* does NULL check */
			}
			/* timestamp */
			{
				SYSTEMTIME st;
				time_t timestamp;
				DateTime_GetSystemtime(GetDlgItem(hwndDlg,IDC_TIME_TIMESTAMP),&st); /* time gets synchronized */
				if (!SystemTimeToTimeStamp(&st,&timestamp))
					timestamp=time(NULL);
				db_set_dw(NULL,"AutoShutdown","TimeStamp",(DWORD)timestamp);
			}
			/* shutdown type */
			{
				int index = SendDlgItemMessage(hwndDlg,IDC_COMBO_SHUTDOWNTYPE,CB_GETCURSEL,0,0);
				if (index != LB_ERR) db_set_b(NULL,"AutoShutdown","ShutdownType",(BYTE)SendDlgItemMessage(hwndDlg,IDC_COMBO_SHUTDOWNTYPE,CB_GETITEMDATA,(WPARAM)index,0));
				index=SendDlgItemMessage(hwndDlg,IDC_COMBO_COUNTDOWNUNIT,CB_GETCURSEL,0,0);
				if (index != LB_ERR) db_set_dw(NULL,"AutoShutdown","CountdownUnit",(DWORD)SendDlgItemMessage(hwndDlg,IDC_COMBO_COUNTDOWNUNIT,CB_GETITEMDATA,(WPARAM)index,0));
				db_set_dw(NULL,"AutoShutdown","Countdown",(DWORD)GetDlgItemInt(hwndDlg,IDC_EDIT_COUNTDOWN,NULL,FALSE));
				db_set_b(NULL,"AutoShutdown","CpuUsageThreshold",(BYTE)GetDlgItemInt(hwndDlg,IDC_EDIT_CPUUSAGE,NULL,FALSE));
			}
			/* watcher type */
			{
				WORD watcherType = (WORD)(IsDlgButtonChecked(hwndDlg,IDC_RADIO_STTIME)?SDWTF_ST_TIME:SDWTF_ST_COUNTDOWN);
				if (IsDlgButtonChecked(hwndDlg,IDC_CHECK_SPECIFICTIME)) watcherType|=SDWTF_SPECIFICTIME;
				if (IsDlgButtonChecked(hwndDlg,IDC_CHECK_MESSAGE)) watcherType|=SDWTF_MESSAGE;
				if (IsDlgButtonChecked(hwndDlg,IDC_CHECK_FILETRANSFER)) watcherType|=SDWTF_FILETRANSFER;
				if (IsDlgButtonChecked(hwndDlg,IDC_CHECK_IDLE)) watcherType|=SDWTF_IDLE;
				if (IsDlgButtonChecked(hwndDlg,IDC_CHECK_STATUS)) watcherType|=SDWTF_STATUS;
				if (IsDlgButtonChecked(hwndDlg,IDC_CHECK_CPUUSAGE)) watcherType|=SDWTF_CPUUSAGE;
				db_set_w(NULL,"AutoShutdown","WatcherFlags",watcherType);
				ServiceStartWatcher(0,watcherType);
			}
			DestroyWindow(hwndDlg);
			return TRUE;
			/* fall through */
		case IDCANCEL: /* WM_CLOSE */
			DestroyWindow(hwndDlg);
			SetShutdownToolbarButton(false);
			SetShutdownMenuItem(false);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/************************* Services ***********************************/

static INT_PTR ServiceShowSettingsDialog(WPARAM wParam,LPARAM lParam)
{
	if (hwndSettingsDlg != NULL) { /* already opened, bring to front */
		SetForegroundWindow(hwndSettingsDlg);
		return 0;
	}
	return (CreateDialog(hInst, MAKEINTRESOURCE(IDD_SETTINGS), NULL, SettingsDlgProc) == NULL);
}

/************************* Toolbar ************************************/

static HANDLE hToolbarButton;

int ToolbarLoaded(WPARAM,LPARAM)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.hIconHandleUp = iconList[2].hIcolib;
	ttb.hIconHandleDn = iconList[1].hIcolib;
	ttb.pszService = "AutoShutdown/MenuCommand";
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
	ttb.name = LPGEN("Start/Stop automatic shutdown");
	ttb.pszTooltipUp = LPGEN("Start automatic shutdown");
	ttb.pszTooltipDn = LPGEN("Stop automatic shutdown");
	hToolbarButton = TopToolbar_AddButton(&ttb);
	return 0;
}

void SetShutdownToolbarButton(bool fActive)
{
	if (hToolbarButton)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hToolbarButton,fActive?TTBST_PUSHED:0);
}

/************************* Menu Item **********************************/

static HGENMENU hMainMenuItem,hTrayMenuItem;

void SetShutdownMenuItem(bool fActive)
{
	/* main menu */
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 2001090000;
	if (fActive)
	{
		mi.icolibItem = iconList[1].hIcolib;
		mi.ptszName = LPGENT("Stop automatic &shutdown");
	}
	else
	{
		mi.icolibItem = iconList[2].hIcolib;
		mi.ptszName = LPGENT("Automatic &shutdown...");
	}
	mi.pszService = "AutoShutdown/MenuCommand";
	mi.flags = CMIF_TCHAR;
	if (hMainMenuItem != NULL) {
		mi.flags |= CMIM_NAME | CMIM_ICON;
		Menu_ModifyItem(hMainMenuItem, &mi);
	}
	else hMainMenuItem = Menu_AddMainMenuItem(&mi);

	/* tray menu */
	mi.position = 899999;
	if (hTrayMenuItem != NULL) {
		mi.flags |= CMIM_NAME | CMIM_ICON;
		Menu_ModifyItem(hTrayMenuItem, &mi);
	}
	else hTrayMenuItem = Menu_AddTrayMenuItem(&mi);
}

static INT_PTR MenuItemCommand(WPARAM,LPARAM)
{
	/* toggle between StopWatcher and ShowSettingsDdialog */
	if (ServiceIsWatcherEnabled(0,0))
		ServiceStopWatcher(0,0);
	else
		ServiceShowSettingsDialog(0,0);
	return 0;
}

/************************* Misc ***************************************/

void InitSettingsDlg(void)
{
	/* Menu Item */
	hServiceMenuCommand = CreateServiceFunction("AutoShutdown/MenuCommand", MenuItemCommand);
	hMainMenuItem=hTrayMenuItem=NULL;
	SetShutdownMenuItem(false);
	/* Hotkey */
	AddHotkey();
	/* Services */
	hwndSettingsDlg=NULL;
	hServiceShowDlg = CreateServiceFunction(MS_AUTOSHUTDOWN_SHOWSETTINGSDIALOG, ServiceShowSettingsDialog);
}

void UninitSettingsDlg(void)
{
	/* Menu Item */
	DestroyServiceFunction(hServiceMenuCommand);
	/* Services */
	DestroyServiceFunction(hServiceShowDlg);
}

