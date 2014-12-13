/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "commonheaders.h"

static DWORD protoModeMsgFlags;
static HWND hwndStatusMsg;

static bool Proto_IsAccountEnabled(PROTOACCOUNT *pa)
{
	return pa && ((pa->bIsEnabled && !pa->bDynDisabled) || pa->bOldProto);
}

static bool Proto_IsAccountLocked(PROTOACCOUNT *pa)
{
	return pa && db_get_b(NULL, pa->szModuleName, "LockMainStatus", 0) != 0;
}

static const TCHAR *GetDefaultMessage(int status)
{
	switch(status) {
		case ID_STATUS_AWAY: return TranslateT("I've been away since %time%.");
		case ID_STATUS_NA: return TranslateT("Give it up, I'm not in!");
		case ID_STATUS_OCCUPIED: return TranslateT("Not right now.");
		case ID_STATUS_DND: return TranslateT("Give a guy some peace, would ya?");
		case ID_STATUS_FREECHAT: return TranslateT("I'm a chatbot!");
		case ID_STATUS_ONLINE: return TranslateT("Yep, I'm here.");
		case ID_STATUS_OFFLINE: return TranslateT("Nope, not here.");
		case ID_STATUS_INVISIBLE: return TranslateT("I'm hiding from the mafia.");
		case ID_STATUS_ONTHEPHONE: return TranslateT("That'll be the phone.");
		case ID_STATUS_OUTTOLUNCH: return TranslateT("Mmm... food.");
		case ID_STATUS_IDLE: return TranslateT("idleeeeeeee");
	}
	return NULL;
}

static const char *StatusModeToDbSetting(int status, const char *suffix)
{
	const char *prefix;
	static char str[64];

	switch(status)
	{
		case ID_STATUS_AWAY:		prefix = "Away";		break;
		case ID_STATUS_NA:			prefix = "Na";			break;
		case ID_STATUS_DND:			prefix = "Dnd";			break;
		case ID_STATUS_OCCUPIED:	prefix = "Occupied";	break;
		case ID_STATUS_FREECHAT:	prefix = "FreeChat";	break;
		case ID_STATUS_ONLINE:		prefix = "On";			break;
		case ID_STATUS_OFFLINE:		prefix = "Off";			break;
		case ID_STATUS_INVISIBLE:	prefix = "Inv";			break;
		case ID_STATUS_ONTHEPHONE:	prefix = "Otp";			break;
		case ID_STATUS_OUTTOLUNCH:	prefix = "Otl";			break;
		case ID_STATUS_IDLE:		prefix = "Idl";			break;
		default: return NULL;
	}
	mir_snprintf(str, SIZEOF(str), "%s%s", prefix, suffix);
	return str;
}

static bool GetStatusModeByte(int status, const char *suffix, bool bDefault = false)
{
	return db_get_b(NULL, "SRAway", StatusModeToDbSetting(status, suffix), bDefault) != 0;
}

static void SetStatusModeByte(int status, const char *suffix, BYTE value)
{
	db_set_b(NULL, "SRAway", StatusModeToDbSetting(status, suffix), value);
}

static TCHAR* GetAwayMessage(int statusMode, char *szProto)
{
	if (szProto && !(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(statusMode)))
		return NULL;

	if ( GetStatusModeByte(statusMode, "Ignore"))
		return NULL;

	DBVARIANT dbv;
	if ( GetStatusModeByte(statusMode, "UsePrev")) {
		if ( db_get_ts(NULL, "SRAway", StatusModeToDbSetting(statusMode, "Msg"), &dbv))
			dbv.ptszVal = mir_tstrdup(GetDefaultMessage(statusMode));
	}
	else {
		if ( db_get_ts(NULL, "SRAway", StatusModeToDbSetting(statusMode, "Default"), &dbv))
			dbv.ptszVal = mir_tstrdup(GetDefaultMessage(statusMode));

		for (int i=0; dbv.ptszVal[i]; i++) {
			if (dbv.ptszVal[i] != '%')
				continue;

			TCHAR substituteStr[128];
			if ( !_tcsnicmp(dbv.ptszVal + i, _T("%time%"), 6)) {
				MIRANDA_IDLE_INFO mii = { sizeof(mii) };
				CallService(MS_IDLE_GETIDLEINFO, 0, (LPARAM)&mii);

				if (mii.idleType == 1) {
					int mm;
					SYSTEMTIME t;
					GetLocalTime(&t);
					mm = t.wMinute + t.wHour * 60 - mii.idleTime;
					if (mm < 0) mm += 60 * 24;
					t.wMinute = mm % 60;
					t.wHour = mm / 60;
					GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &t, NULL, substituteStr, SIZEOF(substituteStr));
				}
				else GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, substituteStr, SIZEOF(substituteStr));
			}
			else if ( !_tcsnicmp(dbv.ptszVal + i, _T("%date%"), 6))
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, substituteStr, SIZEOF(substituteStr));
			else continue;

			if (mir_tstrlen(substituteStr) > 6)
				dbv.ptszVal = (TCHAR*)mir_realloc(dbv.ptszVal, (mir_tstrlen(dbv.ptszVal) + 1 + mir_tstrlen(substituteStr) - 6) * sizeof(TCHAR));
			MoveMemory(dbv.ptszVal + i + mir_tstrlen(substituteStr), dbv.ptszVal + i + 6, (mir_tstrlen(dbv.ptszVal) - i - 5) * sizeof(TCHAR));
			memcpy(dbv.ptszVal+i, substituteStr, mir_tstrlen(substituteStr) * sizeof(TCHAR));
		}
	}
	return dbv.ptszVal;
}

static LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CHAR:
		if (wParam == '\n' && GetKeyState(VK_CONTROL) & 0x8000)
		{
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000) //ctrl-a
		{
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return 0;
		}
		if (wParam == 23 && GetKeyState(VK_CONTROL) & 0x8000) // ctrl-w
		{
			SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
			return 0;
		}
		if (wParam == 127 && GetKeyState(VK_CONTROL) & 0x8000) //ctrl-backspace
		{
			DWORD start, end;
			TCHAR *text;
			int textLen;
			SendMessage(hwnd, EM_GETSEL, (WPARAM)&end, 0);
			SendMessage(hwnd, WM_KEYDOWN, VK_LEFT, 0);
			SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, 0);
			textLen = GetWindowTextLength(hwnd);
			text = (TCHAR *)alloca(sizeof(TCHAR) * (textLen + 1));
			GetWindowText(hwnd, text, textLen + 1);
			memmove(text + start, text + end, sizeof(TCHAR) * (textLen + 1 - end));
			SetWindowText(hwnd, text);
			SendMessage(hwnd, EM_SETSEL, start, start);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM)hwnd);
			return 0;
		}
		break;
	}
	return mir_callNextSubclass(hwnd, MessageEditSubclassProc, msg, wParam, lParam);
}

void ChangeAllProtoMessages(char *szProto, int statusMode, TCHAR *msg)
{
	if (szProto == NULL) {
		int nAccounts;
		PROTOACCOUNT **accounts;
		ProtoEnumAccounts(&nAccounts, &accounts);

		for (int i = 0; i < nAccounts; i++) {
			PROTOACCOUNT *pa = accounts[i];
			if (!Proto_IsAccountEnabled(pa))
				continue;

			if ((CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) && !Proto_IsAccountLocked(pa))
				CallProtoService(pa->szModuleName, PS_SETAWAYMSGT, statusMode, (LPARAM)msg);
		}
	}
	else CallProtoService(szProto, PS_SETAWAYMSGT, statusMode, (LPARAM)msg);
}

struct SetAwayMsgData
{
	int statusMode;
	int countdown;
	TCHAR okButtonFormat[64];
	char *szProto;
	HANDLE hPreshutdown;
};

struct SetAwasMsgNewData
{
	char *szProto;
	int statusMode;
};

#define DM_SRAWAY_SHUTDOWN WM_USER+10

static INT_PTR CALLBACK SetAwayMsgDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	SetAwayMsgData* dat = (SetAwayMsgData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SetAwasMsgNewData *newdat = (SetAwasMsgNewData*)lParam;
			dat = (SetAwayMsgData*)mir_alloc(sizeof(SetAwayMsgData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->statusMode = newdat->statusMode;
			dat->szProto = newdat->szProto;
			mir_free(newdat);
			SendDlgItemMessage(hwndDlg, IDC_MSG, EM_LIMITTEXT, 1024, 0);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_MSG), MessageEditSubclassProc);
			{
				TCHAR str[256], format[128];
				GetWindowText(hwndDlg, format, SIZEOF(format));
				mir_sntprintf(str, SIZEOF(str), format, pcli->pfnGetStatusModeDescription(dat->statusMode, 0));
				SetWindowText(hwndDlg, str);
			}
			GetDlgItemText(hwndDlg, IDOK, dat->okButtonFormat, SIZEOF(dat->okButtonFormat));
			{
				TCHAR *msg = GetAwayMessage(dat->statusMode, dat->szProto);
				SetDlgItemText(hwndDlg, IDC_MSG, msg);
				mir_free(msg);
			}
			dat->countdown = 6;
			SendMessage(hwndDlg, WM_TIMER, 0, 0);
			Window_SetProtoIcon_IcoLib(hwndDlg, dat->szProto, dat->statusMode);
			Utils_RestoreWindowPosition(hwndDlg, NULL, "SRAway", "AwayMsgDlg");
			SetTimer(hwndDlg, 1, 1000, 0);
			dat->hPreshutdown = HookEventMessage(ME_SYSTEM_PRESHUTDOWN, hwndDlg, DM_SRAWAY_SHUTDOWN);
		}
		return TRUE;

	case WM_TIMER:
		if (--dat->countdown >= 0) {
			TCHAR str[64];
			mir_sntprintf(str, SIZEOF(str), dat->okButtonFormat, dat->countdown);
			SetDlgItemText(hwndDlg, IDOK, str);
		}
		else {
			KillTimer(hwndDlg, 1);
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;

	case WM_CLOSE:
		{
			TCHAR *msg = GetAwayMessage(dat->statusMode, dat->szProto);
			ChangeAllProtoMessages(dat->szProto, dat->statusMode, msg);
			mir_free(msg);
		}
		DestroyWindow(hwndDlg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (dat->countdown < 0) {
				TCHAR str[1024];
				GetDlgItemText(hwndDlg, IDC_MSG, str, SIZEOF(str));
				ChangeAllProtoMessages(dat->szProto, dat->statusMode, str);
				db_set_ts(NULL, "SRAway", StatusModeToDbSetting(dat->statusMode, "Msg"), str);
				DestroyWindow(hwndDlg);
			}
			else PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;

		case IDC_MSG:
			if (dat->countdown >= 0) {
				KillTimer(hwndDlg, 1);
				SetDlgItemText(hwndDlg, IDOK, TranslateT("OK"));
				dat->countdown = -1;
			}
			break;
		}
		break;

	case DM_SRAWAY_SHUTDOWN:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg, NULL, "SRAway", "AwayMsgDlg");
		KillTimer(hwndDlg, 1);
		UnhookEvent(dat->hPreshutdown);
		Window_FreeIcon_IcoLib(hwndDlg);
		mir_free(dat);
		hwndStatusMsg = NULL;
		break;
	}
	return FALSE;
}

static int StatusModeChange(WPARAM wParam, LPARAM lParam)
{
	int statusMode = (int)wParam;
	char *szProto = (char*)lParam;

	if (protoModeMsgFlags == 0)
		return 0;

	// If its a global change check the complete PFLAGNUM_3 flags to see if a popup might be needed
	if (!szProto) {
		if (!(protoModeMsgFlags & Proto_Status2Flag(statusMode)))
			return 0;
	}
	else {
		// If its a single protocol check the PFLAGNUM_3 for the single protocol
		if (!(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) ||
			 !(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(statusMode)))
			 return 0;
	}

	BOOL bScreenSaverRunning = IsScreenSaverRunning();
	if (GetStatusModeByte(statusMode, "Ignore"))
		ChangeAllProtoMessages(szProto, statusMode, NULL);

	else if (bScreenSaverRunning || GetStatusModeByte(statusMode, "NoDlg", true)) {
		TCHAR *msg = GetAwayMessage(statusMode, szProto);
		ChangeAllProtoMessages(szProto, statusMode, msg);
		mir_free(msg);
	}
	else {
		SetAwasMsgNewData *newdat = (SetAwasMsgNewData*)mir_alloc(sizeof(SetAwasMsgNewData));
		newdat->szProto = szProto;
		newdat->statusMode = statusMode;
		if (hwndStatusMsg)
			DestroyWindow(hwndStatusMsg);
		hwndStatusMsg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SETAWAYMSG), NULL, SetAwayMsgDlgProc, (LPARAM)newdat);
	}
	return 0;
}

static const int statusModes[] =
{
	ID_STATUS_OFFLINE, ID_STATUS_ONLINE, ID_STATUS_AWAY, ID_STATUS_NA, ID_STATUS_OCCUPIED, ID_STATUS_DND,
	ID_STATUS_FREECHAT, ID_STATUS_INVISIBLE, ID_STATUS_OUTTOLUNCH, ID_STATUS_ONTHEPHONE, ID_STATUS_IDLE
};

struct AwayMsgInfo
{
	int ignore;
	int noDialog;
	int usePrevious;
	TCHAR msg[1024];
};

struct AwayMsgDlgData
{
	struct AwayMsgInfo info[SIZEOF(statusModes)];
	int oldPage;
};

static INT_PTR CALLBACK DlgProcAwayMsgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AwayMsgDlgData *dat = (AwayMsgDlgData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	HWND hLst = GetDlgItem(hwndDlg, IDC_LST_STATUS);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			dat = (AwayMsgDlgData*)mir_alloc(sizeof(AwayMsgDlgData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->oldPage = -1;
			for (int i = 0; i < SIZEOF(statusModes); i++) {
				if (!(protoModeMsgFlags & Proto_Status2Flag(statusModes[i])))
					continue;

				int j;
				if (hLst) {
					j = SendDlgItemMessage(hwndDlg, IDC_LST_STATUS, LB_ADDSTRING, 0, (LPARAM)pcli->pfnGetStatusModeDescription(statusModes[i], 0));
					SendDlgItemMessage(hwndDlg, IDC_LST_STATUS, LB_SETITEMDATA, j, statusModes[i]);
				}
				else {
					j = SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_ADDSTRING, 0, (LPARAM)pcli->pfnGetStatusModeDescription(statusModes[i], 0));
					SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_SETITEMDATA, j, statusModes[i]);
				}

				dat->info[j].ignore = GetStatusModeByte(statusModes[i], "Ignore");
				dat->info[j].noDialog = GetStatusModeByte(statusModes[i], "NoDlg", true);
				dat->info[j].usePrevious = GetStatusModeByte(statusModes[i], "UsePrev");

				DBVARIANT dbv;
				if (db_get_ts(NULL, "SRAway", StatusModeToDbSetting(statusModes[i], "Default"), &dbv))
					if (db_get_ts(NULL, "SRAway", StatusModeToDbSetting(statusModes[i], "Msg"), &dbv))
						dbv.ptszVal = mir_tstrdup(GetDefaultMessage(statusModes[i]));
				mir_tstrcpy(dat->info[j].msg, dbv.ptszVal);
				mir_free(dbv.ptszVal);
			}
			if (hLst)
				SendDlgItemMessage(hwndDlg, IDC_LST_STATUS, LB_SETCURSEL, 0, 0);
			else
				SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_SETCURSEL, 0, 0);
			SendMessage(hwndDlg, WM_COMMAND, hLst ? MAKEWPARAM(IDC_LST_STATUS, LBN_SELCHANGE) : MAKEWPARAM(IDC_STATUS, CBN_SELCHANGE), 0);
		}
		return TRUE;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;
			if (mis->CtlID == IDC_LST_STATUS)
				mis->itemHeight = 20;
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->CtlID != IDC_LST_STATUS) break;
			if (dis->itemID < 0) break;

			TCHAR buf[128];
			SendDlgItemMessage(hwndDlg, IDC_LST_STATUS, LB_GETTEXT, dis->itemID, (LPARAM)buf);

			if (dis->itemState & (ODS_SELECTED | ODS_FOCUS)) {
				FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
				SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			}
			else {
				FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));
				SetTextColor(dis->hDC, GetSysColor(COLOR_WINDOWTEXT));
			}

			RECT rc = dis->rcItem;
			DrawIconEx(dis->hDC, 3, (rc.top + rc.bottom - 16) / 2, LoadSkinnedProtoIcon(NULL, dis->itemData), 16, 16, 0, NULL, DI_NORMAL);
			rc.left += 25;
			SetBkMode(dis->hDC, TRANSPARENT);
			DrawText(dis->hDC, buf, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LST_STATUS:
		case IDC_STATUS:
			if ((HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == LBN_SELCHANGE)) {
				int i = hLst ?
					SendDlgItemMessage(hwndDlg, IDC_LST_STATUS, LB_GETCURSEL, 0, 0) :
					SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_GETCURSEL, 0, 0);
				if (dat->oldPage != -1) {
					dat->info[dat->oldPage].ignore = IsDlgButtonChecked(hwndDlg, IDC_DONTREPLY);
					dat->info[dat->oldPage].noDialog = IsDlgButtonChecked(hwndDlg, IDC_NODIALOG);
					dat->info[dat->oldPage].usePrevious = IsDlgButtonChecked(hwndDlg, IDC_USEPREVIOUS);
					GetDlgItemText(hwndDlg, IDC_MSG, dat->info[dat->oldPage].msg, SIZEOF(dat->info[dat->oldPage].msg));
				}
				CheckDlgButton(hwndDlg, IDC_DONTREPLY, i < 0 ? 0 : dat->info[i].ignore);
				CheckDlgButton(hwndDlg, IDC_NODIALOG, i < 0 ? 0 : dat->info[i].noDialog);
				CheckDlgButton(hwndDlg, IDC_USEPREVIOUS, i < 0 ? 0 : dat->info[i].usePrevious);
				CheckDlgButton(hwndDlg, IDC_USESPECIFIC, i < 0 ? 0 : !dat->info[i].usePrevious);

				SetDlgItemText(hwndDlg, IDC_MSG, i < 0 ? _T("") : dat->info[i].msg);

				EnableWindow(GetDlgItem(hwndDlg, IDC_NODIALOG), i < 0 ? 0 : !dat->info[i].ignore);
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEPREVIOUS), i < 0 ? 0 : !dat->info[i].ignore);
				EnableWindow(GetDlgItem(hwndDlg, IDC_USESPECIFIC), i < 0 ? 0 : !dat->info[i].ignore);
				EnableWindow(GetDlgItem(hwndDlg, IDC_MSG), i < 0 ? 0 : !(dat->info[i].ignore || dat->info[i].usePrevious));
				dat->oldPage = i;
			}
			return 0;

		case IDC_DONTREPLY:
		case IDC_USEPREVIOUS:
		case IDC_USESPECIFIC:
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_STATUS, CBN_SELCHANGE), 0);
			break;

		case IDC_MSG:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return 0;
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_STATUS, CBN_SELCHANGE), 0);
				{
					int i = hLst ? (SendDlgItemMessage(hwndDlg, IDC_LST_STATUS, LB_GETCOUNT, 0, 0) - 1) :
						(SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_GETCOUNT, 0, 0) - 1);
					for (; i >= 0; i--) {
						int status = hLst ?
							SendDlgItemMessage(hwndDlg, IDC_LST_STATUS, LB_GETITEMDATA, i, 0) :
							SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_GETITEMDATA, i, 0);
						SetStatusModeByte(status, "Ignore", (BYTE)dat->info[i].ignore);
						SetStatusModeByte(status, "NoDlg", (BYTE)dat->info[i].noDialog);
						SetStatusModeByte(status, "UsePrev", (BYTE)dat->info[i].usePrevious);
						db_set_ts(NULL, "SRAway", StatusModeToDbSetting(status, "Default"), dat->info[i].msg);
					}
					return TRUE;
				}
			}
			break;
		}
		break;

	case WM_DESTROY:
		mir_free(dat);
		break;
	}
	return FALSE;
}

static int AwayMsgOptInitialise(WPARAM wParam, LPARAM)
{
	if (protoModeMsgFlags == 0)
		return 0;

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 870000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_AWAYMSG);
	odp.pszTitle = LPGEN("Status messages");
	odp.pszGroup = LPGEN("Status");
	odp.pfnDlgProc = DlgProcAwayMsgOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

static int AwayMsgSendModernOptInit(WPARAM wParam, LPARAM)
{
	if (protoModeMsgFlags == 0)
		return 0;

	static const int iBoldControls[] =
	{
		IDC_TXT_TITLE1, IDC_TXT_TITLE2, IDC_TXT_TITLE3,
		MODERNOPT_CTRL_LAST
	};

	MODERNOPTOBJECT obj = { 0 };
	obj.cbSize = sizeof(obj);
	obj.hInstance = hInst;
	obj.dwFlags = MODEROPT_FLG_TCHAR | MODEROPT_FLG_NORESIZE;
	obj.iSection = MODERNOPT_PAGE_STATUS;
	obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
	obj.iBoldControls = (int*)iBoldControls;
	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_STATUS);
	obj.pfnDlgProc = DlgProcAwayMsgOpts;
	obj.lpzHelpUrl = "http://wiki.miranda-ng.org/";
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
	return 0;
}

static int AwayMsgSendAccountsChanged(WPARAM, LPARAM)
{
	protoModeMsgFlags = 0;

	int nAccounts;
	PROTOACCOUNT** accounts;
	ProtoEnumAccounts(&nAccounts, &accounts);
	for (int i = 0; i < nAccounts; i++) {
		if (!Proto_IsAccountEnabled(accounts[i]))
			continue;

		protoModeMsgFlags |= CallProtoService(accounts[i]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0);
	}

	return 0;
}

static int AwayMsgSendModulesLoaded(WPARAM, LPARAM)
{
	AwayMsgSendAccountsChanged(0, 0);

	HookEvent(ME_CLIST_STATUSMODECHANGE, StatusModeChange);
	HookEvent(ME_MODERNOPT_INITIALIZE, AwayMsgSendModernOptInit);
	HookEvent(ME_OPT_INITIALISE, AwayMsgOptInitialise);
	return 0;
}

//remember to mir_free() the return value
static INT_PTR sttGetAwayMessageT(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)GetAwayMessage((int)wParam, (char*)lParam);
}

int LoadAwayMessageSending(void)
{
	HookEvent(ME_SYSTEM_MODULESLOADED, AwayMsgSendModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, AwayMsgSendAccountsChanged);

	CreateServiceFunction(MS_AWAYMSG_GETSTATUSMSGW, sttGetAwayMessageT);
	return 0;
}
