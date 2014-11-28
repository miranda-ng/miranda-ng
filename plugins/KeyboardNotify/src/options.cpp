/*

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

#include "Common.h"

#define IDI_BLANK			200

void LoadSettings(void);
int InitializeOptions(WPARAM,LPARAM);
INT_PTR CALLBACK DlgProcOptions(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcProtoOptions(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcBasicOptions(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcEffectOptions(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcThemeOptions(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcProcesses(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcEventLeds(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcXstatusList(HWND, UINT, WPARAM, LPARAM);
void exportThemes(const TCHAR *);
void importThemes(const TCHAR *, BOOL);
void writeThemeToCombo(const TCHAR *, const TCHAR *, BOOL);
void createProcessListAux(void);
void destroyProcessListAux(void);
void createXstatusListAux(void);
void destroyXstatusListAux(void);


extern HINSTANCE hInst;

extern BYTE bFlashOnMsg;
extern BYTE bFlashOnURL;
extern BYTE bFlashOnFile;
extern BYTE bFlashOnOther;
extern BYTE bFullScreenMode;
extern BYTE bScreenSaverRunning;
extern BYTE bWorkstationLocked;
extern BYTE bProcessesAreRunning;
extern BYTE bWorkstationActive;
extern BYTE bFlashIfMsgOpen;
extern BYTE bFlashIfMsgOlder;
extern WORD wSecondsOlder;
extern BYTE bFlashUntil;
extern WORD wBlinksNumber;
extern BYTE bMirandaOrWindows;
extern WORD wStatusMap;
extern WORD wReminderCheck;
extern BYTE bFlashLed[3];
extern BYTE bFlashEffect;
extern BYTE bSequenceOrder;
extern WORD wCustomTheme;
extern WORD wStartDelay;
extern BYTE bFlashSpeed;
extern BYTE bEmulateKeypresses;
extern BYTE bOverride;
extern BYTE bFlashIfMsgWinNotTop;
extern BYTE bTrillianLedsMsg;
extern BYTE bTrillianLedsURL;
extern BYTE bTrillianLedsFile;
extern BYTE bTrillianLedsOther;

extern PROTOCOL_LIST ProtoList;
extern PROCESS_LIST ProcessList;

HWND hwndProto, hwndBasic, hwndEffect, hwndTheme, hwndIgnore, hwndCurrentTab;

TCHAR *AttendedName[]={_T("Miranda"), _T("Windows")};
TCHAR *OrderName[]={TranslateT("left->right"), TranslateT("right->left"), TranslateT("left<->right")};

PROCESS_LIST ProcessListAux;
XSTATUS_INFO *XstatusListAux;
BYTE trillianLedsMsg, trillianLedsURL, trillianLedsFile, trillianLedsOther;

// **
// ** Initialize the Miranda options page
// **
int InitializeOptions(WPARAM wParam,LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle = LPGEN("Keyboard Flash");
	odp.pszGroup = LPGEN("Events");
	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(wParam, &odp);
	return 0;
}

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			HWND tc;
			TCITEM tci;
			tc = GetDlgItem(hwndDlg, IDC_TABS);
			tci.mask = TCIF_TEXT;
			tci.pszText = TranslateT("Accounts");
			TabCtrl_InsertItem(tc, 0, &tci);
			tci.pszText = TranslateT("Rules");
			TabCtrl_InsertItem(tc, 1, &tci);
			tci.pszText = TranslateT("Flashing");
			TabCtrl_InsertItem(tc, 2, &tci);
			tci.pszText = TranslateT("Themes");
			TabCtrl_InsertItem(tc, 3, &tci);
			tci.pszText = TranslateT("Ignore");
			TabCtrl_InsertItem(tc, 4, &tci);

			hwndProto = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_PROTO_OPTIONS), hwndDlg, DlgProcProtoOptions, (LPARAM) NULL);
			EnableThemeDialogTexture(hwndProto, ETDT_ENABLETAB);
			SetWindowPos(hwndProto, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			ShowWindow(hwndProto, SW_SHOW);
			hwndBasic = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_BASIC_OPTIONS), hwndDlg, DlgProcBasicOptions, (LPARAM) NULL);
			EnableThemeDialogTexture(hwndBasic, ETDT_ENABLETAB);
			SetWindowPos(hwndBasic, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			hwndEffect = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EFFECT_OPTIONS), hwndDlg, DlgProcEffectOptions, (LPARAM) NULL);
			EnableThemeDialogTexture(hwndEffect, ETDT_ENABLETAB);
			SetWindowPos(hwndEffect, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			hwndTheme = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_THEME_OPTIONS), hwndDlg, DlgProcThemeOptions, (LPARAM) NULL);
			EnableThemeDialogTexture(hwndTheme, ETDT_ENABLETAB);
			SetWindowPos(hwndTheme, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			hwndIgnore = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_IGNORE_OPTIONS), hwndDlg, DlgProcIgnoreOptions, (LPARAM) NULL);
			EnableThemeDialogTexture(hwndIgnore, ETDT_ENABLETAB);
			SetWindowPos(hwndIgnore, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			hwndCurrentTab = hwndProto;
			return TRUE;

		}
	case WM_COMMAND:
		break;
	case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->code) {
			case TCN_SELCHANGE:
				switch (wParam) {
				case IDC_TABS:
					{
						HWND hwnd;
						switch (TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TABS))) {
						default:
						case 0:
							hwnd = hwndProto;
							break;
						case 1:
							hwnd = hwndBasic;
							break;
						case 2:
							hwnd = hwndEffect;
							break;
						case 3:
							hwnd = hwndTheme;
							break;
						case 4:
							hwnd = hwndIgnore;
							break;
						}
						if (hwnd!=hwndCurrentTab) {
							ShowWindow(hwnd, SW_SHOW);
							ShowWindow(hwndCurrentTab, SW_HIDE);
							hwndCurrentTab = hwnd;
						}
					}
					break;
				}
				break;
			case PSN_APPLY:
				SendMessage(hwndProto, WM_NOTIFY, wParam, lParam);
				SendMessage(hwndBasic, WM_NOTIFY, wParam, lParam);
				SendMessage(hwndEffect, WM_NOTIFY, wParam, lParam);
				SendMessage(hwndTheme, WM_NOTIFY, wParam, lParam);
				SendMessage(hwndIgnore, WM_NOTIFY, wParam, lParam);
				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}


INT_PTR CALLBACK DlgProcProtoOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL initDlg=FALSE;

	switch (msg) {

		case WM_INITDIALOG:
			initDlg=TRUE;
			TranslateDialogDefault(hwndDlg);

			// proto list
			{	
				LVCOLUMN lvCol;
				LVITEM lvItem;
				HWND hList = GetDlgItem(hwndDlg, IDC_PROTOCOLLIST);

				// create columns
				ListView_SetExtendedListViewStyleEx(hList, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
				memset(&lvCol, 0, sizeof(lvCol));
				lvCol.mask = LVCF_WIDTH|LVCF_TEXT;
				lvCol.pszText = TranslateT("Protocol");
				lvCol.cx = 118;
				ListView_InsertColumn(hList, 0, &lvCol);
				// fill
				memset(&lvItem, 0, sizeof(lvItem));
				lvItem.mask = LVIF_TEXT|LVIF_PARAM;
				lvItem.cchTextMax = 256;
				lvItem.iItem = 0;
				lvItem.iSubItem = 0;
				for (int i=0; i < ProtoList.protoCount; i++) {
					int count; PROTOACCOUNT** protos;
					ProtoEnumAccounts( &count, &protos );
					if(ProtoList.protoInfo[i].visible) {
						lvItem.lParam = (LPARAM)ProtoList.protoInfo[i].szProto;
						lvItem.pszText = protos[i] -> tszAccountName;
						ListView_InsertItem(hList, &lvItem);
						ListView_SetCheckState(hList, lvItem.iItem, ProtoList.protoInfo[i].enabled);
						lvItem.iItem++;
					}
				}
			}

			initDlg=FALSE;
			return TRUE;

		case WM_NOTIFY:
			{
				//Here we have pressed either the OK or the APPLY button.
				switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code) {
					case PSN_APPLY:
						// enabled protos
						{
							int i;
							LVITEM lvItem;
							HWND hList = GetDlgItem(hwndDlg, IDC_PROTOCOLLIST);

							memset(&lvItem, 0, sizeof(lvItem));
							lvItem.mask = LVIF_PARAM;
							lvItem.iSubItem = 0;
							for (i=0; i < ListView_GetItemCount(hList); i++) {
								lvItem.iItem = i;
								ListView_GetItem(hList, &lvItem);
								db_set_b(NULL, KEYBDMODULE, (char *)lvItem.lParam, (BYTE)!!ListView_GetCheckState(hList, lvItem.iItem));
							}
						}

						LoadSettings();

						return TRUE;
					} // switch code - 0
					break;
				case IDC_PROTOCOLLIST:
					switch(((NMHDR*)lParam)->code) {
					case LVN_ITEMCHANGED:
						{
							NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;

							if (!initDlg && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK))
								SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
						}
						break;
					} // switch code - IDC_PROTOCOLLIST
					break;
				} //switch idFrom
			}
			break; //End WM_NOTIFY

			default:
			break;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcBasicOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL initDlg=FALSE;

	switch (msg) {

		case WM_INITDIALOG:
			initDlg=TRUE;
			TranslateDialogDefault(hwndDlg);

			createProcessListAux();
			createXstatusListAux();

			CheckDlgButton(hwndDlg, IDC_ONMESSAGE, bFlashOnMsg ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ONURL, bFlashOnURL ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ONFILE, bFlashOnFile ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ONOTHER, bFlashOnOther ? BST_CHECKED:BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_FSCREEN, bFullScreenMode ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SSAVER, bScreenSaverRunning ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOCKED, bWorkstationLocked ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PGMS, bProcessesAreRunning ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ACTIVE, bWorkstationActive ? BST_CHECKED:BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_IFOPEN, bFlashIfMsgOpen ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_IFNOTTOP, bFlashIfMsgWinNotTop ? BST_CHECKED:BST_UNCHECKED);
			if (!bFlashIfMsgOpen)
				EnableWindow(GetDlgItem(hwndDlg, IDC_IFNOTTOP), FALSE);
			CheckDlgButton(hwndDlg, IDC_IFOLDER, bFlashIfMsgOlder ? BST_CHECKED:BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_OLDERSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SOLDER), 0);
			SendDlgItemMessage(hwndDlg, IDC_OLDERSPIN, UDM_SETRANGE32, 1, MAKELONG(UD_MAXVAL, 0));
			SendDlgItemMessage(hwndDlg, IDC_OLDERSPIN, UDM_SETPOS, 0, MAKELONG(wSecondsOlder, 0));
			if (!bFlashIfMsgOlder) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_SOLDER), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OLDERSPIN), FALSE);
			}

			CheckDlgButton(hwndDlg, IDC_UNTILBLK, bFlashUntil&UNTIL_NBLINKS ? BST_CHECKED:BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SBLINK), 0);
			SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETRANGE32, 1, MAKELONG(UD_MAXVAL, 0));
			SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETPOS, 0, MAKELONG(wBlinksNumber, 0));
			if (!(bFlashUntil & UNTIL_NBLINKS)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_SBLINK), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKSPIN), FALSE);
			}
			CheckDlgButton(hwndDlg, IDC_UNTILATTENDED, bFlashUntil&UNTIL_REATTENDED ? BST_CHECKED:BST_UNCHECKED);
			for (int i = 0; i < 2; i++) {
				int index = SendDlgItemMessage(hwndDlg, IDC_MIRORWIN, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)AttendedName[i]);
				if (index != CB_ERR && index != CB_ERRSPACE)
					SendDlgItemMessage(hwndDlg, IDC_MIRORWIN, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
			}
			SendDlgItemMessage(hwndDlg, IDC_MIRORWIN, CB_SETCURSEL, (WPARAM)bMirandaOrWindows, 0);
			if (!(bFlashUntil & UNTIL_REATTENDED))
				EnableWindow(GetDlgItem(hwndDlg, IDC_MIRORWIN), FALSE);
			CheckDlgButton(hwndDlg, IDC_UNTILOPEN, bFlashUntil&UNTIL_EVENTSOPEN ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_UNTILCOND, bFlashUntil&UNTIL_CONDITIONS ? BST_CHECKED:BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_ONLINE, wStatusMap&MAP_ONLINE ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AWAY, wStatusMap&MAP_AWAY ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NA, wStatusMap&MAP_NA ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OCCUPIED, wStatusMap&MAP_OCCUPIED ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DND, wStatusMap&MAP_DND ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FREECHAT, wStatusMap&MAP_FREECHAT ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_INVISIBLE, wStatusMap&MAP_INVISIBLE ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ONTHEPHONE, wStatusMap&MAP_ONTHEPHONE ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OUTTOLUNCH, wStatusMap&MAP_OUTTOLUNCH ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OFFLINE, wStatusMap&MAP_OFFLINE ? BST_CHECKED:BST_UNCHECKED);

			SendDlgItemMessage(hwndDlg, IDC_REMCHECK, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SREMCHECK), 0);
			SendDlgItemMessage(hwndDlg, IDC_REMCHECK, UDM_SETRANGE32, 0, MAKELONG(UD_MAXVAL, 0));
			SendDlgItemMessage(hwndDlg, IDC_REMCHECK, UDM_SETPOS, 0, MAKELONG(wReminderCheck, 0));

			initDlg=FALSE;
			return TRUE;

		case WM_VSCROLL:
		case WM_HSCROLL:
			SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
			break;

		case WM_DESTROY:
			destroyProcessListAux();
			destroyXstatusListAux();
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_ONMESSAGE:
			case IDC_ONURL:
			case IDC_ONFILE:
			case IDC_ONOTHER:
			case IDC_IFOPEN:
			case IDC_IFNOTTOP:
			case IDC_IFOLDER:
			case IDC_UNTILBLK:
			case IDC_UNTILATTENDED:
			case IDC_UNTILOPEN:
			case IDC_UNTILCOND:
			case IDC_FSCREEN:
			case IDC_SSAVER:
			case IDC_LOCKED:
			case IDC_PGMS:
			case IDC_ACTIVE:
			case IDC_ONLINE:
			case IDC_AWAY:
			case IDC_NA:
			case IDC_OCCUPIED:
			case IDC_DND:
			case IDC_FREECHAT:
			case IDC_INVISIBLE:
			case IDC_ONTHEPHONE:
			case IDC_OUTTOLUNCH:
			case IDC_OFFLINE:
				EnableWindow(GetDlgItem(hwndDlg, IDC_IFNOTTOP), IsDlgButtonChecked(hwndDlg, IDC_IFOPEN) == BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SOLDER), IsDlgButtonChecked(hwndDlg, IDC_IFOLDER) == BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OLDERSPIN), IsDlgButtonChecked(hwndDlg, IDC_IFOLDER) == BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SBLINK), IsDlgButtonChecked(hwndDlg, IDC_UNTILBLK) == BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKSPIN), IsDlgButtonChecked(hwndDlg, IDC_UNTILBLK) == BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_MIRORWIN), IsDlgButtonChecked(hwndDlg, IDC_UNTILATTENDED) == BST_CHECKED);
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_SOLDER:
				if(HIWORD(wParam) == EN_CHANGE && !initDlg) {
					BOOL translated;
					int val = GetDlgItemInt(hwndDlg, IDC_SOLDER, &translated, FALSE);
					if (translated && val < 1)
						SendDlgItemMessage(hwndDlg, IDC_OLDERSPIN, UDM_SETPOS, 0, MAKELONG(val, 0));
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				}
				return TRUE;
			case IDC_SBLINK:
				if(HIWORD(wParam) == EN_CHANGE && !initDlg) {
					BOOL translated;
					int val = GetDlgItemInt(hwndDlg, IDC_SBLINK, &translated, FALSE);
					if (translated && val < 1)
						SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETPOS, 0, MAKELONG(val, 0));
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				}
				return TRUE;
			case IDC_ASSIGNPGMS:
				if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PROCESSES), hwndDlg, DlgProcProcesses, 0) == IDC_OKPGM)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_SELECTXSTATUS:
				if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_XSTATUSES), hwndDlg, DlgProcXstatusList, 0) == IDC_OKXST)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_SREMCHECK:
				if(HIWORD(wParam) == EN_CHANGE && !initDlg)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_MIRORWIN:
				if(HIWORD(wParam) == CBN_SELCHANGE)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			}
			break;

		case WM_NOTIFY:
			{
				BYTE untilMap = 0;
				WORD statusMap = 0;
				//Here we have pressed either the OK or the APPLY button.
				switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code) {
					case PSN_APPLY:
						db_set_b(NULL, KEYBDMODULE, "onmsg", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_ONMESSAGE) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "onurl", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_ONURL) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "onfile", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_ONFILE) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "onother", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_ONOTHER) == BST_CHECKED ? 1:0));

						db_set_b(NULL, KEYBDMODULE, "fscreenmode", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_FSCREEN) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "ssaverrunning", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SSAVER) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "wstationlocked", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_LOCKED) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "procsrunning", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_PGMS) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "wstationactive", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_ACTIVE) == BST_CHECKED ? 1:0));

						db_set_b(NULL, KEYBDMODULE, "ifmsgopen", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_IFOPEN) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "ifmsgnottop", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_IFNOTTOP) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "ifmsgolder", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_IFOLDER) == BST_CHECKED ? 1:0));
						db_set_w(NULL, KEYBDMODULE, "secsolder", (WORD)SendDlgItemMessage(hwndDlg, IDC_OLDERSPIN, UDM_GETPOS, 0, 0));

						if(IsDlgButtonChecked(hwndDlg, IDC_UNTILBLK) == BST_CHECKED)
							untilMap |= UNTIL_NBLINKS;
						if (IsDlgButtonChecked(hwndDlg, IDC_UNTILATTENDED) == BST_CHECKED)
							untilMap |= UNTIL_REATTENDED;
						if (IsDlgButtonChecked(hwndDlg, IDC_UNTILOPEN) == BST_CHECKED)
							untilMap |= UNTIL_EVENTSOPEN;
						if (IsDlgButtonChecked(hwndDlg, IDC_UNTILCOND) == BST_CHECKED)
							untilMap |= UNTIL_CONDITIONS;
						db_set_b(NULL, KEYBDMODULE, "funtil", untilMap);
						db_set_w(NULL, KEYBDMODULE, "nblinks", (WORD)SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_GETPOS, 0, 0));
						db_set_b(NULL, KEYBDMODULE, "mirorwin", (BYTE)SendDlgItemMessage(hwndDlg, IDC_MIRORWIN, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_MIRORWIN, CB_GETCURSEL, 0, 0), 0));

						if(IsDlgButtonChecked(hwndDlg, IDC_ONLINE) == BST_CHECKED)
							statusMap |= MAP_ONLINE;
						if(IsDlgButtonChecked(hwndDlg, IDC_AWAY) == BST_CHECKED)
							statusMap |= MAP_AWAY;
						if(IsDlgButtonChecked(hwndDlg, IDC_NA) == BST_CHECKED)
							statusMap |= MAP_NA;
						if(IsDlgButtonChecked(hwndDlg, IDC_OCCUPIED) == BST_CHECKED)
							statusMap |= MAP_OCCUPIED;
						if(IsDlgButtonChecked(hwndDlg, IDC_DND) == BST_CHECKED)
							statusMap |= MAP_DND;
						if(IsDlgButtonChecked(hwndDlg, IDC_FREECHAT) == BST_CHECKED)
							statusMap |= MAP_FREECHAT;
						if(IsDlgButtonChecked(hwndDlg, IDC_INVISIBLE) == BST_CHECKED)
							statusMap |= MAP_INVISIBLE;
						if(IsDlgButtonChecked(hwndDlg, IDC_ONTHEPHONE) == BST_CHECKED)
							statusMap |= MAP_ONTHEPHONE;
						if(IsDlgButtonChecked(hwndDlg, IDC_OUTTOLUNCH) == BST_CHECKED)
							statusMap |= MAP_OUTTOLUNCH;
						if(IsDlgButtonChecked(hwndDlg, IDC_OFFLINE) == BST_CHECKED)
							statusMap |= MAP_OFFLINE;
						db_set_w(NULL, KEYBDMODULE, "status", statusMap);

						db_set_w(NULL, KEYBDMODULE, "remcheck", (WORD)SendDlgItemMessage(hwndDlg, IDC_REMCHECK, UDM_GETPOS, 0, 0));

						int i = 0;
						for (int j = 0; j < ProcessListAux.count; j++)
							if (ProcessListAux.szFileName[j])
								db_set_ts(NULL, KEYBDMODULE, fmtDBSettingName("process%d", i++), ProcessListAux.szFileName[j]);
						db_set_w(NULL, KEYBDMODULE, "processcount", (WORD)i);
						while (!db_unset(NULL, KEYBDMODULE, fmtDBSettingName("process%d", i++)));

						if (XstatusListAux)
							for (int i = 0; i < ProtoList.protoCount; i++)
								for (int j = 0; j < (int)XstatusListAux[i].count; j++)
									db_set_b(NULL, KEYBDMODULE, fmtDBSettingName("%sxstatus%d", ProtoList.protoInfo[i].szProto, j), (BYTE)XstatusListAux[i].enabled[j]);

						LoadSettings();

						return TRUE;
					} // switch code
					break;
				} //switch idFrom
			}
			break; //End WM_NOTIFY
					
			default:
			break;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcEffectOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;
	DBVARIANT dbv;
	static BOOL initDlg=FALSE;

	switch (msg) {

		case WM_INITDIALOG:
			initDlg=TRUE;
			TranslateDialogDefault(hwndDlg);

			trillianLedsMsg = bTrillianLedsMsg;
			trillianLedsURL = bTrillianLedsURL;
			trillianLedsFile = bTrillianLedsFile;
			trillianLedsOther = bTrillianLedsOther;

			CheckDlgButton(hwndDlg, IDC_NUM, bFlashLed[0] ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CAPS, bFlashLed[1] ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SCROLL, bFlashLed[2] ? BST_CHECKED:BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_SAMETIME, bFlashEffect == FLASH_SAMETIME ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_INTURN, bFlashEffect == FLASH_INTURN ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_INSEQUENCE, bFlashEffect == FLASH_INSEQUENCE ? BST_CHECKED:BST_UNCHECKED);
			for (i=0; i < 3; i++) {
				int index = SendDlgItemMessage(hwndDlg, IDC_SEQORDER, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)(OrderName[i]));
				if (index != CB_ERR && index != CB_ERRSPACE)
					SendDlgItemMessage(hwndDlg, IDC_SEQORDER, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
			}
			SendDlgItemMessage(hwndDlg, IDC_SEQORDER, CB_SETCURSEL, (WPARAM)bSequenceOrder, 0);
			if (bFlashEffect != FLASH_INSEQUENCE)
				EnableWindow(GetDlgItem(hwndDlg, IDC_SEQORDER), FALSE);
			CheckDlgButton(hwndDlg, IDC_CUSTOM, bFlashEffect == FLASH_CUSTOM ? BST_CHECKED:BST_UNCHECKED);
			for (i=0; !db_get_ts(NULL, KEYBDMODULE, fmtDBSettingName("theme%d", i), &dbv); i++) {
				int index = SendDlgItemMessage(hwndDlg, IDC_SCUSTOM, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)dbv.ptszVal);
				db_free(&dbv);
				if (index != CB_ERR && index != CB_ERRSPACE)
					SendDlgItemMessage(hwndDlg, IDC_SCUSTOM, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
			}
			SendDlgItemMessage(hwndDlg, IDC_SCUSTOM, CB_SETCURSEL, (WPARAM)wCustomTheme, 0);
			if (bFlashEffect != FLASH_CUSTOM)
				EnableWindow(GetDlgItem(hwndDlg, IDC_SCUSTOM), FALSE);
			CheckDlgButton(hwndDlg, IDC_TRILLIAN, bFlashEffect == FLASH_TRILLIAN ? BST_CHECKED:BST_UNCHECKED);
			if (bFlashEffect != FLASH_TRILLIAN)
				EnableWindow(GetDlgItem(hwndDlg, IDC_ASSIGNLEDS), FALSE);

			SendDlgItemMessage(hwndDlg, IDC_DELAYSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SDELAY), 0);
			SendDlgItemMessage(hwndDlg, IDC_DELAYSPIN, UDM_SETRANGE32, 0, MAKELONG(UD_MAXVAL, 0));
			SendDlgItemMessage(hwndDlg, IDC_DELAYSPIN, UDM_SETPOS, 0, MAKELONG(wStartDelay, 0));

			SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETRANGE, FALSE, MAKELONG(0, 5));
			SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETPOS, TRUE, bFlashSpeed);

			CheckDlgButton(hwndDlg, IDC_KEYPRESSES, bEmulateKeypresses ? BST_CHECKED:BST_UNCHECKED);

			initDlg=FALSE;
			return TRUE;

		case WM_VSCROLL:
		case WM_HSCROLL:
			SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
			break;

		case WM_DESTROY:
			previewFlashing(FALSE);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_NUM:
			case IDC_CAPS:
			case IDC_SCROLL:
			case IDC_SAMETIME:
			case IDC_INTURN:
			case IDC_INSEQUENCE:
			case IDC_CUSTOM:
			case IDC_TRILLIAN:
			case IDC_SPEED:
			case IDC_KEYPRESSES:
				EnableWindow(GetDlgItem(hwndDlg, IDC_SEQORDER), IsDlgButtonChecked(hwndDlg, IDC_INSEQUENCE) == BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SCUSTOM), IsDlgButtonChecked(hwndDlg, IDC_CUSTOM) == BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ASSIGNLEDS), IsDlgButtonChecked(hwndDlg, IDC_TRILLIAN) == BST_CHECKED);
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_ASSIGNLEDS:
				if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EVENTLEDS), hwndDlg, DlgProcEventLeds, 0) == IDC_OK)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_SDELAY:
				if(HIWORD(wParam) == EN_CHANGE && !initDlg)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_PREVIEW:
				previewFlashing(IsDlgButtonChecked(hwndDlg, IDC_PREVIEW) == BST_CHECKED);
				return TRUE;
			case IDC_SEQORDER:
			case IDC_SCUSTOM:
				if(HIWORD(wParam) == CBN_SELCHANGE)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			}
			break;

		case WM_NOTIFY:
			{
				//Here we have pressed either the OK or the APPLY button.
				switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code) {
					case PSN_APPLY:
						db_set_b(NULL, KEYBDMODULE, "fnum", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NUM) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "fcaps", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CAPS) == BST_CHECKED ? 1:0));
						db_set_b(NULL, KEYBDMODULE, "fscroll", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SCROLL) == BST_CHECKED ? 1:0));
		
						if(IsDlgButtonChecked(hwndDlg, IDC_INTURN) == BST_CHECKED)
							db_set_b(NULL, KEYBDMODULE, "feffect", FLASH_INTURN);
						else if (IsDlgButtonChecked(hwndDlg, IDC_INSEQUENCE) == BST_CHECKED)
							db_set_b(NULL, KEYBDMODULE, "feffect", FLASH_INSEQUENCE);
						else if (IsDlgButtonChecked(hwndDlg, IDC_CUSTOM) == BST_CHECKED)
							db_set_b(NULL, KEYBDMODULE, "feffect", FLASH_CUSTOM);
						else if (IsDlgButtonChecked(hwndDlg, IDC_TRILLIAN) == BST_CHECKED)
							db_set_b(NULL, KEYBDMODULE, "feffect", FLASH_TRILLIAN);
						else
							db_set_b(NULL, KEYBDMODULE, "feffect", FLASH_SAMETIME);
						db_set_b(NULL, KEYBDMODULE, "order", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SEQORDER, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_SEQORDER, CB_GETCURSEL, 0, 0), 0));
						db_set_w(NULL, KEYBDMODULE, "custom", (WORD)SendDlgItemMessage(hwndDlg, IDC_SCUSTOM, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_SCUSTOM, CB_GETCURSEL, 0, 0), 0));

						db_set_b(NULL, KEYBDMODULE, "ledsmsg", trillianLedsMsg);
						db_set_b(NULL, KEYBDMODULE, "ledsfile", trillianLedsFile);
						db_set_b(NULL, KEYBDMODULE, "ledsurl", trillianLedsURL);
						db_set_b(NULL, KEYBDMODULE, "ledsother", trillianLedsOther);

						db_set_w(NULL, KEYBDMODULE, "sdelay", (WORD)SendDlgItemMessage(hwndDlg, IDC_DELAYSPIN, UDM_GETPOS, 0, 0));

						db_set_b(NULL, KEYBDMODULE, "speed", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_GETPOS, 0, 0));

						db_set_b(NULL, KEYBDMODULE, "keypresses", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_KEYPRESSES) == BST_CHECKED ? 1:0));

						LoadSettings();

						return TRUE;
					} // switch code
					break;
				} //switch idFrom
			}
			break; //End WM_NOTIFY
					
			default:
			break;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcThemeOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;
	TCHAR *str;
	DBVARIANT dbv;
	static BOOL initDlg=FALSE;

	switch (msg) {

		case WM_INITDIALOG:
			initDlg=TRUE;
			TranslateDialogDefault(hwndDlg);

			SendDlgItemMessage(hwndDlg, IDC_THEME, EM_LIMITTEXT, MAX_PATH, 0);
			SendDlgItemMessage(hwndDlg, IDC_CUSTOMSTRING, EM_LIMITTEXT, MAX_PATH, 0);

			for (i=0; !db_get_ts(NULL, KEYBDMODULE, fmtDBSettingName("theme%d", i), &dbv); i++) {
				int index = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)dbv.ptszVal);
				db_free(&dbv);
				if (index != CB_ERR && index != CB_ERRSPACE) {
					str = (TCHAR *)malloc((MAX_PATH+1)*sizeof(TCHAR));
					if (str)
						if (db_get_ts(NULL, KEYBDMODULE, fmtDBSettingName("custom%d", i), &dbv))
							str[0] = _T('\0');
						else {
							_tcscpy(str, dbv.ptszVal);
							db_free(&dbv);
						}
					SendDlgItemMessage(hwndDlg, IDC_THEME, CB_SETITEMDATA, (WPARAM)index, (LPARAM)str);
				}
			}

			EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
			if (SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETCOUNT, 0, 0) == 0)
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
			else {
				SendDlgItemMessage(hwndDlg, IDC_THEME, CB_SETCURSEL, (WPARAM)wCustomTheme, 0);
				str = (TCHAR *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)wCustomTheme, 0);
				if (str)
					SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str);
			}

			CheckDlgButton(hwndDlg, IDC_OVERRIDE, bOverride ? BST_CHECKED:BST_UNCHECKED);

			initDlg=FALSE;
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_THEME:
				switch (HIWORD(wParam)) {
				int item;
				TCHAR theme[MAX_PATH+1];

				case CBN_SELENDOK:
				case CBN_SELCHANGE:
					str = (TCHAR *)SendMessage((HWND)lParam, CB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);
					if (str)
						SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str);
					else
						SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, _T(""));
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
					break;
				case CBN_EDITCHANGE:
					GetDlgItemText(hwndDlg, IDC_THEME, theme, SIZEOF(theme));
					if ((item = SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, -1, (LPARAM)theme)) == CB_ERR) {	//new theme
						SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, _T(""));
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
					} else {
						str = (TCHAR *)SendMessage((HWND)lParam, CB_GETITEMDATA, (WPARAM)item, 0);
						if (str)
							SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str);
						else
							SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, _T(""));
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
					}
					EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
					break;
				}
				return TRUE;
			case IDC_CUSTOMSTRING:
				if(HIWORD(wParam) == EN_CHANGE) {
					TCHAR theme[MAX_PATH+1], customAux[MAX_PATH+1];

					GetDlgItemText(hwndDlg, IDC_THEME, theme, SIZEOF(theme));
					int item = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_FINDSTRINGEXACT, -1, (LPARAM)theme);
					if (item == CB_ERR)
						return TRUE;
					str = (TCHAR *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)item, 0);
					if (str) {
						GetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, customAux, MAX_PATH);
						if (wcscmp(str, customAux))
							EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), TRUE);
						else
							EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
					}
				}
				return TRUE;
			case IDC_TEST:
				{
				TCHAR custom[MAX_PATH+1];

				GetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, custom, MAX_PATH);
				SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, normalizeCustomString(custom));
				testSequence(custom);
				}
				return TRUE;
			case IDC_ADD:
				{
				TCHAR theme[MAX_PATH+1];

				GetDlgItemText(hwndDlg, IDC_THEME, theme, SIZEOF(theme));
				if (!theme[0])
					return TRUE;
				int item = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_ADDSTRING, 0, (LPARAM)theme);
				str = (TCHAR *)malloc((MAX_PATH+1)*sizeof(TCHAR));
				if (str) {
					GetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str, MAX_PATH);
					SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, normalizeCustomString(str));
				}
				SendDlgItemMessage(hwndDlg, IDC_THEME, CB_SETITEMDATA, (WPARAM)item, (LPARAM)str);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
				}
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_UPDATE:
				{
				TCHAR theme[MAX_PATH+1];

				GetDlgItemText(hwndDlg, IDC_THEME, theme, SIZEOF(theme));
				int item = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_FINDSTRINGEXACT, -1, (LPARAM)theme);
				str = (TCHAR *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)item, 0);
				if (str) {
					GetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str, MAX_PATH);
					SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, normalizeCustomString(str));
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
				}
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_DELETE:
				{
				TCHAR theme[MAX_PATH+1];

				GetDlgItemText(hwndDlg, IDC_THEME, theme, SIZEOF(theme));
				int item = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_FINDSTRINGEXACT, -1, (LPARAM)theme);
				str = (TCHAR *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)item, 0);
				if (str)
					free(str);
				SendDlgItemMessage(hwndDlg, IDC_THEME, CB_DELETESTRING, (WPARAM)item, 0);
				if (SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETCOUNT, 0, 0) == 0) {
					SetDlgItemText(hwndDlg, IDC_THEME, _T(""));
					SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, _T(""));
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
				} else {
					SendDlgItemMessage(hwndDlg, IDC_THEME, CB_SETCURSEL, 0, 0);
					str = (TCHAR *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, 0, 0);
					if (str)
						SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str);
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
				}
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_EXPORT:
				{
				TCHAR path[MAX_PATH+1], filter[MAX_PATH+1], *pfilter;
				OPENFILENAME ofn={0};

				path[0] = _T('\0');
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hwndDlg;
				ofn.hInstance = NULL;
				_tcscpy(filter, TranslateT("Keyboard Notify Theme"));
				wcscat(filter, _T(" (*.knt)"));
				pfilter = filter + _tcslen(filter) + 1;
				_tcscpy(pfilter, _T("*.knt"));
				pfilter = pfilter + _tcslen(pfilter) + 1;
				_tcscpy(pfilter, TranslateT("All Files"));
				pfilter = pfilter + _tcslen(pfilter) + 1;
				_tcscpy(pfilter, _T("*.*"));
				pfilter = pfilter + _tcslen(pfilter) + 1;
				*pfilter = _T('\0');  
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = path;
				ofn.Flags = OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_NOREADONLYRETURN|OFN_PATHMUSTEXIST;
				ofn.nMaxFile = SIZEOF(path);
				ofn.lpstrDefExt = _T("knt");
				if(GetSaveFileName(&ofn))
					exportThemes(path);
				}
				return TRUE;
			case IDC_IMPORT:
				{
				TCHAR path[MAX_PATH+1], filter[MAX_PATH+1], *pfilter;
				OPENFILENAME ofn={0};

				path[0] = _T('\0');
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hwndDlg;
				ofn.hInstance = NULL;
				_tcscpy(filter, TranslateT("Keyboard Notify Theme"));
				wcscat(filter, _T(" (*.knt)"));
				pfilter = filter + _tcslen(filter) + 1;
				_tcscpy(pfilter, _T("*.knt"));
				pfilter = pfilter + _tcslen(pfilter) + 1;
				_tcscpy(pfilter, TranslateT("All Files"));
				pfilter = pfilter + _tcslen(pfilter) + 1;
				_tcscpy(pfilter, _T("*.*"));
				pfilter = pfilter + _tcslen(pfilter) + 1;
				*pfilter = _T('\0');  
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = path;
				ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR;
				ofn.nMaxFile = SIZEOF(path);
				ofn.lpstrDefExt = _T("knt");
				if(GetOpenFileName(&ofn)) {
					importThemes(path, IsDlgButtonChecked(hwndDlg, IDC_OVERRIDE) == BST_CHECKED);
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				}
				}
				return TRUE;
			case IDC_OVERRIDE:
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				return TRUE;
			}
			break;

		case WM_NOTIFY:
			{
				int count;
				TCHAR theme[MAX_PATH+1], themeAux[MAX_PATH+1], *str;
				//Here we have pressed either the OK or the APPLY button.
				switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code) {
					case PSN_APPLY:
						if (!db_get_ts(NULL, KEYBDMODULE, fmtDBSettingName("theme%d", wCustomTheme), &dbv))
							_tcscpy(theme, dbv.ptszVal);
						else
							theme[0] = _T('\0');

						// Here we will delete all the items in the theme combo on the Flashing tab: we will load them again later
						for (i=0; SendDlgItemMessage(hwndEffect, IDC_SCUSTOM, CB_DELETESTRING, 0, (LPARAM)i) != CB_ERR; i++);

						count = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETCOUNT, 0, 0);
						for (i=0, wCustomTheme=0; i < count; i++) {
							SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETLBTEXT, (WPARAM)i, (LPARAM)themeAux);
							db_set_ts(NULL, KEYBDMODULE, fmtDBSettingName("theme%d", i), themeAux);
							str = (TCHAR *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)i, 0);
							if (str)
								db_set_ts(NULL, KEYBDMODULE, fmtDBSettingName("custom%d", i), str);
							else
								db_set_ts(NULL, KEYBDMODULE, fmtDBSettingName("custom%d", i), _T(""));

							if (!wcscmp(theme, themeAux))
								wCustomTheme = i;

							// Here we will update the theme combo on the Flashing tab: horrible but can't imagine a better way right now
							SendDlgItemMessage(hwndEffect, IDC_SCUSTOM, CB_INSERTSTRING, (WPARAM)i, (LPARAM)themeAux);
							SendDlgItemMessage(hwndEffect, IDC_SCUSTOM, CB_SETITEMDATA, (WPARAM)i, (LPARAM)i);
						}
						for (i=count; !db_unset(NULL, KEYBDMODULE, fmtDBSettingName("theme%d", i)); i++)
							db_unset(NULL, KEYBDMODULE, fmtDBSettingName("custom%d", i));

						db_set_w(NULL, KEYBDMODULE, "custom", wCustomTheme);
						// Still updating here the the Flashing tab's controls
						SendDlgItemMessage(hwndEffect, IDC_SCUSTOM, CB_SETCURSEL, (WPARAM)wCustomTheme, 0);

						db_set_b(NULL, KEYBDMODULE, "override", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_OVERRIDE) == BST_CHECKED ? 1:0));

						return TRUE;
					} // switch code
					break;
				} //switch idFrom
			}
			break; //End WM_NOTIFY

			case WM_DESTROY:
			{
			int item, count = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETCOUNT, 0, 0);

			for (item=0; item < count; item++) {
				str = (TCHAR *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)item, 0);
				if (str)
					free(str);
			}
			break;
			}
					
			default:
			break;
	}

	return FALSE;
}


void exportThemes(const TCHAR *filename)
{
	int i;
	FILE *fExport = _tfopen(filename, _T("wt"));
	DBVARIANT dbv;

	if (!fExport)
		return;

	fwprintf(fExport, TranslateT("\n; Automatically generated Keyboard Notify Theme file\n\n\n"));

	for (i=0; !db_get_ts(NULL, KEYBDMODULE, fmtDBSettingName("theme%d", i), &dbv); i++) {
		_ftprintf(fExport, _T("[%s]\n"), dbv.ptszVal);
		db_free(&dbv);
		if (db_get_ts(NULL, KEYBDMODULE, fmtDBSettingName("custom%d", i), &dbv))
			_ftprintf(fExport, _T("0\n\n"));
		else {
			_ftprintf(fExport, _T("%s\n\n"), dbv.ptszVal);
			db_free(&dbv);
		}
	}

	_ftprintf(fExport, TranslateT("\n; End of automatically generated Keyboard Notify Theme file\n"));

	fclose(fExport);
}


void importThemes(const TCHAR *filename, BOOL overrideExisting)
{
	FILE *fImport = _tfopen(filename, _T("rt"));
	if (!fImport)
		return;

	int status=0;
	size_t i;
	TCHAR buffer[MAX_PATH+1], theme[MAX_PATH+1], *str;

	while (_fgetts(buffer, MAX_PATH, fImport) != NULL) {
		for (str=buffer; *str && isspace(*str); str++); //ltrim
		if (!*str || *str == ';') //empty line or comment
			continue;
		for (i=_tcslen(str)-1; isspace(str[i]); str[i--]='\0'); //rtrim
		switch (status) {
			case 0:
				if (i > 1 && str[0] == '[' && str[i] == ']') {
					status = 1;
					_tcscpy(theme, str+1);
					theme[i-1] = '\0';
				}
				break;
			case 1:
				status = 0;
				writeThemeToCombo(theme, normalizeCustomString(str), overrideExisting);
				break;
		}
	}

	fclose(fImport);
}


void writeThemeToCombo(const TCHAR *theme, const TCHAR *custom, BOOL overrideExisting)
{
	TCHAR *str;

	int item = SendDlgItemMessage(hwndTheme, IDC_THEME, CB_FINDSTRINGEXACT, -1, (LPARAM)theme);
	if (item == CB_ERR) {
		item = SendDlgItemMessage(hwndTheme, IDC_THEME, CB_ADDSTRING, 0, (LPARAM)theme);
		str = (TCHAR *)malloc((MAX_PATH+1)*sizeof(TCHAR));
		if (str)
			_tcscpy(str, custom);
		SendDlgItemMessage(hwndTheme, IDC_THEME, CB_SETITEMDATA, (WPARAM)item, (LPARAM)str);
	} else
		if (overrideExisting) {
			str = (TCHAR *)SendDlgItemMessage(hwndTheme, IDC_THEME, CB_GETITEMDATA, (WPARAM)item, 0);
			if (str)
				_tcscpy(str, custom);
		}
}


INT_PTR CALLBACK DlgProcProcesses(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);

			SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, EM_LIMITTEXT, MAX_PATH, 0);

			for (int i = 0; i < ProcessListAux.count; i++)
				if (ProcessListAux.szFileName[i]) {
					int index = SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)ProcessListAux.szFileName[i]);
					if (index != CB_ERR && index != CB_ERRSPACE)
						SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
				}

			EnableWindow(GetDlgItem(hwndDlg, IDC_ADDPGM), FALSE);
			if (SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_GETCOUNT, 0, 0) == 0)
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), FALSE);
			else
				SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_SETCURSEL, 0, 0);

			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_PROGRAMS:
				switch (HIWORD(wParam)) {
				int item;
				TCHAR szFileName[MAX_PATH+1];

				case CBN_SELENDOK:
				case CBN_SELCHANGE:
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDPGM), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), TRUE);
					break;
				case CBN_EDITCHANGE:
					GetDlgItemText(hwndDlg, IDC_PROGRAMS, szFileName, SIZEOF(szFileName));
					if ((item = SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, -1, (LPARAM)szFileName)) == CB_ERR) {	//new program
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDPGM), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), FALSE);
					} else {
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDPGM), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), TRUE);
					}
					break;
				}
				break;
			case IDC_ADDPGM:
				{
				int item;
				TCHAR szFileName[MAX_PATH+1];

				GetDlgItemText(hwndDlg, IDC_PROGRAMS, szFileName, SIZEOF(szFileName));
				if (!szFileName[0])
					break;
				item = SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_ADDSTRING, 0, (LPARAM)szFileName);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADDPGM), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), TRUE);
				}
				break;
			case IDC_DELETEPGM:
				{
				int item;
				TCHAR szFileName[MAX_PATH+1];

				GetDlgItemText(hwndDlg, IDC_PROGRAMS, szFileName, SIZEOF(szFileName));
				item = SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_FINDSTRINGEXACT, -1, (LPARAM)szFileName);
				SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_DELETESTRING, (WPARAM)item, 0);
				if (SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_GETCOUNT, 0, 0) == 0) {
					SetDlgItemText(hwndDlg, IDC_PROGRAMS, _T(""));
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), FALSE);
				} else
					SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_SETCURSEL, 0, 0);
				}
				break;
			case IDC_OKPGM:
				destroyProcessListAux();

				ProcessListAux.count = SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_GETCOUNT, 0, 0);
				ProcessListAux.szFileName = (TCHAR **)malloc(ProcessListAux.count * sizeof(TCHAR *));
				if (!ProcessListAux.szFileName)
					ProcessListAux.count = 0;
				else
					for (int i = 0; i < ProcessListAux.count; i++) {
						TCHAR szFileNameAux[MAX_PATH+1];

						SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_GETLBTEXT, (WPARAM)i, (LPARAM)szFileNameAux);
						ProcessListAux.szFileName[i] = (TCHAR *)malloc((_tcslen(szFileNameAux) + 1)*sizeof(TCHAR));
						if (ProcessListAux.szFileName[i])
							_tcscpy(ProcessListAux.szFileName[i], szFileNameAux);
					}
	
			case IDC_CANCELPGM:
				EndDialog(hwndDlg, LOWORD(wParam));
				break;
			}
			break;
	}

	return FALSE;
}


void createProcessListAux(void)
{
	ProcessListAux.count = ProcessList.count;
	ProcessListAux.szFileName = (TCHAR **)malloc(ProcessListAux.count * sizeof(TCHAR *));
	if (!ProcessListAux.szFileName)
		ProcessListAux.count = 0;
	else
		for (int i = 0; i < ProcessListAux.count; i++)
			if (!ProcessList.szFileName[i])
				ProcessListAux.szFileName[i] = NULL;
			else {
				ProcessListAux.szFileName[i] = (TCHAR *)malloc((_tcslen(ProcessList.szFileName[i]) + 1)*sizeof(TCHAR));
				if (ProcessListAux.szFileName[i])
					_tcscpy(ProcessListAux.szFileName[i], ProcessList.szFileName[i]);
			}

}


void destroyProcessListAux(void)
{
	if (ProcessListAux.szFileName == NULL)
		return;
	for (int i = 0; i < ProcessListAux.count; i ++) {
		if (ProcessListAux.szFileName[i]) {
			free(ProcessListAux.szFileName[i]);
		}
	}
	free(ProcessListAux.szFileName);
	ProcessListAux.count = 0;
	ProcessListAux.szFileName = NULL;
}


INT_PTR CALLBACK DlgProcEventLeds(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);

			CheckDlgButton(hwndDlg, IDC_MSGLEDNUM, trillianLedsMsg&2 ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MSGLEDCAPS, trillianLedsMsg&4 ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MSGLEDSCROLL, trillianLedsMsg&1 ? BST_CHECKED:BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_FILELEDNUM, trillianLedsFile&2 ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FILELEDCAPS, trillianLedsFile&4 ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FILELEDSCROLL, trillianLedsFile&1 ? BST_CHECKED:BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_URLLEDNUM, trillianLedsURL&2 ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_URLLEDCAPS, trillianLedsURL&4 ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_URLLEDSCROLL, trillianLedsURL&1 ? BST_CHECKED:BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_OTHERLEDNUM, trillianLedsOther&2 ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OTHERLEDCAPS, trillianLedsOther&4 ? BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OTHERLEDSCROLL, trillianLedsOther&1 ? BST_CHECKED:BST_UNCHECKED);

			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_OK:
				trillianLedsMsg = 0;
				if(IsDlgButtonChecked(hwndDlg, IDC_MSGLEDNUM) == BST_CHECKED)
					trillianLedsMsg |= 2;
				if(IsDlgButtonChecked(hwndDlg, IDC_MSGLEDCAPS) == BST_CHECKED)
					trillianLedsMsg |= 4;
				if(IsDlgButtonChecked(hwndDlg, IDC_MSGLEDSCROLL) == BST_CHECKED)
					trillianLedsMsg |= 1;

				trillianLedsFile = 0;
				if(IsDlgButtonChecked(hwndDlg, IDC_FILELEDNUM) == BST_CHECKED)
					trillianLedsFile |= 2;
				if(IsDlgButtonChecked(hwndDlg, IDC_FILELEDCAPS) == BST_CHECKED)
					trillianLedsFile |= 4;
				if(IsDlgButtonChecked(hwndDlg, IDC_FILELEDSCROLL) == BST_CHECKED)
					trillianLedsFile |= 1;

				trillianLedsURL = 0;
				if(IsDlgButtonChecked(hwndDlg, IDC_URLLEDNUM) == BST_CHECKED)
					trillianLedsURL |= 2;
				if(IsDlgButtonChecked(hwndDlg, IDC_URLLEDCAPS) == BST_CHECKED)
					trillianLedsURL |= 4;
				if(IsDlgButtonChecked(hwndDlg, IDC_URLLEDSCROLL) == BST_CHECKED)
					trillianLedsURL |= 1;

				trillianLedsOther = 0;
				if(IsDlgButtonChecked(hwndDlg, IDC_OTHERLEDNUM) == BST_CHECKED)
					trillianLedsOther |= 2;
				if(IsDlgButtonChecked(hwndDlg, IDC_OTHERLEDCAPS) == BST_CHECKED)
					trillianLedsOther |= 4;
				if(IsDlgButtonChecked(hwndDlg, IDC_OTHERLEDSCROLL) == BST_CHECKED)
					trillianLedsOther |= 1;

			case IDC_CANCEL:
				EndDialog(hwndDlg, LOWORD(wParam));
				break;
			}
			break;
	}

	return FALSE;
}


void createXstatusListAux(void)
{
	XstatusListAux = (XSTATUS_INFO *)malloc(ProtoList.protoCount * sizeof(XSTATUS_INFO));
	if (XstatusListAux)
		for (int i = 0; i < ProtoList.protoCount; i++) {
			XstatusListAux[i].count = ProtoList.protoInfo[i].xstatus.count;
			if (!XstatusListAux[i].count)
				XstatusListAux[i].enabled = NULL;
			else {
				XstatusListAux[i].enabled = (BOOL *)malloc(XstatusListAux[i].count * sizeof(BOOL));
				if (!XstatusListAux[i].enabled)
					XstatusListAux[i].count = 0;
				else
					for (int j = 0; j < (int)XstatusListAux[i].count; j++)
						XstatusListAux[i].enabled[j] = ProtoList.protoInfo[i].xstatus.enabled[j];
			}
		}

}


void destroyXstatusListAux(void)
{
	if (XstatusListAux) {
		for (int i = 0; i < ProtoList.protoCount; i++)
			if (XstatusListAux[i].enabled)
				free(XstatusListAux[i].enabled);
		free(XstatusListAux);
		XstatusListAux = NULL;
	}

}


INT_PTR CALLBACK DlgProcXstatusList(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

		case WM_INITDIALOG:

		{
			WPARAM j;
			TVINSERTSTRUCT tvis={0};
			TVITEM tvi={0};
			HTREEITEM hSectionItem, hItem;
			HWND hwndTree = GetDlgItem(hwndDlg, IDC_TREE_XSTATUS);

			TranslateDialogDefault(hwndDlg);
			SetWindowLongPtr(hwndTree, GWL_STYLE, GetWindowLongPtr(hwndTree, GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);

			if (!XstatusListAux) return TRUE;

			// Calculate hImageList size
			int imageCount=1;
			for (int i = 0; i < ProtoList.protoCount; i++)
				if (ProtoList.protoInfo[i].enabled && XstatusListAux[i].count)
					imageCount += XstatusListAux[i].count;
	
			HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, imageCount, imageCount);
			TreeView_SetImageList(hwndTree, hImageList, TVSIL_NORMAL);

			HICON hIconAux=(HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, 0, 0, 0);
			if (hIconAux) {
				ImageList_AddIcon(hImageList, hIconAux);
				DestroyIcon(hIconAux);
			}

			TreeView_SelectItem(hwndTree, NULL);
			ShowWindow(hwndTree, SW_HIDE);
			TreeView_DeleteAllItems(hwndTree);

			for (int i = 0; i < ProtoList.protoCount; i++)
				if (ProtoList.protoInfo[i].enabled && XstatusListAux[i].count) {
					int count;
					PROTOACCOUNT **protos;
					ProtoEnumAccounts( &count, &protos );

					tvis.hParent = NULL;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_STATE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
					tvis.item.pszText = protos[i] -> tszAccountName; 
					tvis.item.lParam = (LPARAM)i;
					tvis.item.stateMask = TVIS_BOLD|TVIS_EXPANDED;
					tvis.item.state = TVIS_BOLD|TVIS_EXPANDED;
					tvis.item.iImage = tvis.item.iSelectedImage = ImageList_AddIcon(hImageList, hIconAux=(HICON)CallProtoService(ProtoList.protoInfo[i].szProto, PS_LOADICON, PLI_PROTOCOL, 0));
					if (hIconAux) DestroyIcon(hIconAux);
					HTREEITEM hParent = TreeView_InsertItem(hwndTree, &tvis);
					for (j = 0; j < XstatusListAux[i].count; j++) {
						tvis.hParent = hParent;
						tvis.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
						if (!j){
							tvis.item.pszText = TranslateT("None"); }
						else {
							TCHAR szDefaultName[1024];
							CUSTOM_STATUS xstatus={0};
							xstatus.cbSize = sizeof(CUSTOM_STATUS);
							xstatus.flags = CSSF_MASK_NAME|CSSF_DEFAULT_NAME|CSSF_TCHAR;
							xstatus.ptszName = szDefaultName;
							xstatus.wParam = &j;
							CallProtoService(ProtoList.protoInfo[i].szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus);
							tvis.item.pszText = TranslateTS(szDefaultName);
						}
						tvis.item.lParam = (LPARAM)j;
						tvis.item.iImage = tvis.item.iSelectedImage = j?ImageList_AddIcon(hImageList, hIconAux=(HICON)CallProtoService(ProtoList.protoInfo[i].szProto, PS_GETCUSTOMSTATUSICON, (WPARAM)j, 0)):0;
						if (hIconAux) DestroyIcon(hIconAux);
						TreeView_InsertItem(hwndTree, &tvis);
					}
				}

			tvi.mask = TVIF_HANDLE|TVIF_PARAM;
			for (hSectionItem=TreeView_GetRoot(hwndTree); hSectionItem; hSectionItem=TreeView_GetNextSibling(hwndTree, hSectionItem)) {
				tvi.hItem = hSectionItem;
				TreeView_GetItem(hwndTree, &tvi);
				unsigned int i = (unsigned int)tvi.lParam;
				TreeView_SetItemState(hwndTree, hSectionItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);
				for (hItem=TreeView_GetChild(hwndTree, hSectionItem); hItem; hItem=TreeView_GetNextSibling(hwndTree, hItem)) {
					tvi.hItem = hItem;
					TreeView_GetItem(hwndTree, &tvi);
					j = (unsigned int)tvi.lParam;
					TreeView_SetItemState(hwndTree, hItem, INDEXTOSTATEIMAGEMASK(XstatusListAux[i].enabled[j]?2:1), TVIS_STATEIMAGEMASK);
				}
			}

			ShowWindow(hwndTree, SW_SHOW);
			TreeView_SetItemState(hwndTree, 0, TVIS_SELECTED, TVIS_SELECTED);
			return TRUE;
		}

		case WM_DESTROY:
		{
			// Destroy tree view imagelist since it does not get destroyed automatically (see msdn docs)
			HIMAGELIST hImageList = TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_TREE_XSTATUS), TVSIL_STATE);
			if (hImageList) {
				TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_TREE_XSTATUS), NULL, TVSIL_STATE);
				ImageList_Destroy(hImageList);
			}
			hImageList = TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_TREE_XSTATUS), TVSIL_NORMAL);
			if (hImageList) {
				TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_TREE_XSTATUS), NULL, TVSIL_NORMAL);
				ImageList_Destroy(hImageList);
			}
			return TRUE;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_OKXST:
			{
				unsigned int i, j;
				HTREEITEM hSectionItem, hItem;
				TVITEM tvi={0};
				HWND hwndTree = GetDlgItem(hwndDlg, IDC_TREE_XSTATUS);

				tvi.mask = TVIF_HANDLE|TVIF_PARAM;
				for (hSectionItem=TreeView_GetRoot(hwndTree); hSectionItem; hSectionItem=TreeView_GetNextSibling(hwndTree, hSectionItem)) {
					tvi.hItem = hSectionItem;
					TreeView_GetItem(hwndTree, &tvi);
					i = (unsigned int)tvi.lParam;
					for (hItem=TreeView_GetChild(hwndTree, hSectionItem); hItem; hItem=TreeView_GetNextSibling(hwndTree, hItem)) {
						tvi.hItem = hItem;
						TreeView_GetItem(hwndTree, &tvi);
						j = (unsigned int)tvi.lParam;
						XstatusListAux[i].enabled[j] = !!(TreeView_GetItemState(hwndTree, hItem, TVIS_STATEIMAGEMASK)&INDEXTOSTATEIMAGEMASK(2));
					}
				}
			}

			case IDC_CANCELXST:
				EndDialog(hwndDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}

	return FALSE;
}
