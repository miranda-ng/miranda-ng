/*
	Name: NewEventNotify - Plugin for Miranda IM
	File: options.c - Manages Option Dialogs and Settings
	Version: 0.0.4
	Description: Notifies you about some events
	Author: icebreaker, <icebreaker@newmail.net>
	Date: 22.07.02 13:06 / Update: 16.09.02 17:45
	Copyright: (C) 2002 Starzinger Michael

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "neweventnotify.h"

PLUGIN_OPTIONS* options;
BOOL bWmNotify;

int OptionsRead(void)
{
	options->bDisable = db_get_b(NULL, MODULE, OPT_DISABLE, FALSE);
	options->bPreview = db_get_b(NULL, MODULE, OPT_PREVIEW, TRUE);
	options->bMenuitem = db_get_b(NULL, MODULE, OPT_MENUITEM, FALSE);
	options->bDefaultColorMsg = db_get_b(NULL, MODULE, OPT_COLDEFAULT_MESSAGE, FALSE);
	options->bDefaultColorUrl = db_get_b(NULL, MODULE, OPT_COLDEFAULT_URL, FALSE);
	options->bDefaultColorFile = db_get_b(NULL, MODULE, OPT_COLDEFAULT_FILE, FALSE);
	options->bDefaultColorOthers = db_get_b(NULL, MODULE, OPT_COLDEFAULT_OTHERS, FALSE);
	options->colBackMsg = (COLORREF)db_get_dw(NULL, MODULE, OPT_COLBACK_MESSAGE, DEFAULT_COLBACK);
	options->colTextMsg = (COLORREF)db_get_dw(NULL, MODULE, OPT_COLTEXT_MESSAGE, DEFAULT_COLTEXT);
	options->colBackUrl = (COLORREF)db_get_dw(NULL, MODULE, OPT_COLBACK_URL, DEFAULT_COLBACK);
	options->colTextUrl = (COLORREF)db_get_dw(NULL, MODULE, OPT_COLTEXT_URL, DEFAULT_COLTEXT);
	options->colBackFile = (COLORREF)db_get_dw(NULL, MODULE, OPT_COLBACK_FILE, DEFAULT_COLBACK);
	options->colTextFile = (COLORREF)db_get_dw(NULL, MODULE, OPT_COLTEXT_FILE, DEFAULT_COLTEXT);
	options->colBackOthers = (COLORREF)db_get_dw(NULL, MODULE, OPT_COLBACK_OTHERS, DEFAULT_COLBACK);
	options->colTextOthers = (COLORREF)db_get_dw(NULL, MODULE, OPT_COLTEXT_OTHERS, DEFAULT_COLTEXT);
	options->maskNotify = (UINT)db_get_b(NULL, MODULE, OPT_MASKNOTIFY, DEFAULT_MASKNOTIFY);
	options->maskActL = (UINT)db_get_b(NULL, MODULE, OPT_MASKACTL, DEFAULT_MASKACTL);
	options->maskActR = (UINT)db_get_b(NULL, MODULE, OPT_MASKACTR, DEFAULT_MASKACTR);
	options->maskActTE = (UINT)db_get_b(NULL, MODULE, OPT_MASKACTTE, DEFAULT_MASKACTE);
	options->bMsgWindowCheck = db_get_b(NULL, MODULE, OPT_MSGWINDOWCHECK, TRUE);
	options->bMsgReplyWindow = db_get_b(NULL, MODULE, OPT_MSGREPLYWINDOW, FALSE);
	options->bMergePopup = db_get_b(NULL, MODULE, OPT_MERGEPOPUP, TRUE);
	options->iDelayMsg = (int)db_get_dw(NULL, MODULE, OPT_DELAY_MESSAGE, DEFAULT_DELAY);
	options->iDelayUrl = (int)db_get_dw(NULL, MODULE, OPT_DELAY_URL, DEFAULT_DELAY);
	options->iDelayFile = (int)db_get_dw(NULL, MODULE, OPT_DELAY_FILE, DEFAULT_DELAY);
	options->iDelayOthers = (int)db_get_dw(NULL, MODULE, OPT_DELAY_OTHERS, DEFAULT_DELAY);
	options->iDelayDefault = (int)DBGetContactSettingRangedWord(NULL, "Popup", "Seconds", SETTING_LIFETIME_DEFAULT, SETTING_LIFETIME_MIN, SETTING_LIFETIME_MAX);
	options->bShowDate = db_get_b(NULL, MODULE, OPT_SHOW_DATE, TRUE);
	options->bShowTime = db_get_b(NULL, MODULE, OPT_SHOW_TIME, TRUE);
	options->bShowHeaders = db_get_b(NULL, MODULE, OPT_SHOW_HEADERS, TRUE);
	options->iNumberMsg = (BYTE)db_get_b(NULL, MODULE, OPT_NUMBER_MSG, TRUE);
	options->bShowON = db_get_b(NULL, MODULE, OPT_SHOW_ON, TRUE);
	options->bHideSend = db_get_b(NULL, MODULE, OPT_HIDESEND, TRUE);
	options->bNoRSS = db_get_b(NULL, MODULE, OPT_NORSS, FALSE);
	options->bReadCheck = db_get_b(NULL, MODULE, OPT_READCHECK, FALSE);
	return 0;
}

int OptionsWrite(void)
{
	db_set_b(NULL, MODULE, OPT_DISABLE, options->bDisable);
	db_set_b(NULL, MODULE, OPT_PREVIEW, options->bPreview);
	db_set_b(NULL, MODULE, OPT_MENUITEM, options->bMenuitem);
	db_set_b(NULL, MODULE, OPT_COLDEFAULT_MESSAGE, options->bDefaultColorMsg);
	db_set_b(NULL, MODULE, OPT_COLDEFAULT_URL, options->bDefaultColorUrl);
	db_set_b(NULL, MODULE, OPT_COLDEFAULT_FILE, options->bDefaultColorFile);
	db_set_b(NULL, MODULE, OPT_COLDEFAULT_OTHERS, options->bDefaultColorOthers);
	db_set_dw(NULL, MODULE, OPT_COLBACK_MESSAGE, (DWORD)options->colBackMsg);
	db_set_dw(NULL, MODULE, OPT_COLTEXT_MESSAGE, (DWORD)options->colTextMsg);
	db_set_dw(NULL, MODULE, OPT_COLBACK_URL, (DWORD)options->colBackUrl);
	db_set_dw(NULL, MODULE, OPT_COLTEXT_URL, (DWORD)options->colTextUrl);
	db_set_dw(NULL, MODULE, OPT_COLBACK_FILE, (DWORD)options->colBackFile);
	db_set_dw(NULL, MODULE, OPT_COLTEXT_FILE, (DWORD)options->colTextFile);
	db_set_dw(NULL, MODULE, OPT_COLBACK_OTHERS, (DWORD)options->colBackOthers);
	db_set_dw(NULL, MODULE, OPT_COLTEXT_OTHERS, (DWORD)options->colTextOthers);
	db_set_b(NULL, MODULE, OPT_MASKNOTIFY, (BYTE)options->maskNotify);
	db_set_b(NULL, MODULE, OPT_MASKACTL, (BYTE)options->maskActL);
	db_set_b(NULL, MODULE, OPT_MASKACTR, (BYTE)options->maskActR);
	db_set_b(NULL, MODULE, OPT_MASKACTTE, (BYTE)options->maskActTE);
	db_set_b(NULL, MODULE, OPT_MSGWINDOWCHECK, options->bMsgWindowCheck);
	db_set_b(NULL, MODULE, OPT_MSGREPLYWINDOW, options->bMsgReplyWindow);
	db_set_b(NULL, MODULE, OPT_MERGEPOPUP, options->bMergePopup);
	db_set_dw(NULL, MODULE, OPT_DELAY_MESSAGE, (DWORD)options->iDelayMsg);
	db_set_dw(NULL, MODULE, OPT_DELAY_URL, (DWORD)options->iDelayUrl);
	db_set_dw(NULL, MODULE, OPT_DELAY_FILE, (DWORD)options->iDelayFile);
	db_set_dw(NULL, MODULE, OPT_DELAY_OTHERS, (DWORD)options->iDelayOthers);
	db_set_b(NULL, MODULE, OPT_SHOW_DATE, options->bShowDate);
	db_set_b(NULL, MODULE, OPT_SHOW_TIME, options->bShowTime);
	db_set_b(NULL, MODULE, OPT_SHOW_HEADERS, options->bShowHeaders);
	db_set_b(NULL, MODULE, OPT_NUMBER_MSG, (BYTE)options->iNumberMsg);
	db_set_b(NULL, MODULE, OPT_SHOW_ON, options->bShowON);
	db_set_b(NULL, MODULE, OPT_HIDESEND, options->bHideSend);
	db_set_b(NULL, MODULE, OPT_NORSS, options->bNoRSS);
	db_set_b(NULL, MODULE, OPT_READCHECK, options->bReadCheck);
	return 0;
}

static void SetCheckBoxState(HWND hWnd, int iCtrl, BOOL bState)
{
	CheckDlgButton(hWnd, iCtrl, bState ? BST_CHECKED : BST_UNCHECKED);
}

static void EnableDlgItem(HWND hWnd, int iCtrl, BOOL bEnable)
{
	EnableWindow(GetDlgItem(hWnd, iCtrl), bEnable);
}

static void UpdateOptionsDlgItemsState(HWND hWnd)
{
	//disable color picker when using default colors
	EnableDlgItem(hWnd, IDC_COLBACK_MESSAGE, !options->bDefaultColorMsg);
	EnableDlgItem(hWnd, IDC_COLTEXT_MESSAGE, !options->bDefaultColorMsg);
	EnableDlgItem(hWnd, IDC_COLBACK_URL, !options->bDefaultColorUrl);
	EnableDlgItem(hWnd, IDC_COLTEXT_URL, !options->bDefaultColorUrl);
	EnableDlgItem(hWnd, IDC_COLBACK_FILE, !options->bDefaultColorFile);
	EnableDlgItem(hWnd, IDC_COLTEXT_FILE, !options->bDefaultColorFile);
	EnableDlgItem(hWnd, IDC_COLBACK_OTHERS, !options->bDefaultColorOthers);
	EnableDlgItem(hWnd, IDC_COLTEXT_OTHERS, !options->bDefaultColorOthers);
	//disable merge messages options when is not using
	EnableDlgItem(hWnd, IDC_CHKSHOWDATE, options->bMergePopup);
	EnableDlgItem(hWnd, IDC_CHKSHOWTIME, options->bMergePopup);
	EnableDlgItem(hWnd, IDC_CHKSHOWHEADERS, options->bMergePopup);
	EnableDlgItem(hWnd, IDC_CMDEDITHEADERS, options->bMergePopup && options->bShowHeaders);
	EnableDlgItem(hWnd, IDC_NUMBERMSG, options->bMergePopup);
	EnableDlgItem(hWnd, IDC_LBNUMBERMSG, options->bMergePopup);
	EnableDlgItem(hWnd, IDC_RDNEW, options->bMergePopup && options->iNumberMsg);
	EnableDlgItem(hWnd, IDC_RDOLD, options->bMergePopup && options->iNumberMsg);
	//disable delay textbox when infinite is checked
	EnableDlgItem(hWnd, IDC_DELAY_MESSAGE, options->iDelayMsg != -1);
	EnableDlgItem(hWnd, IDC_DELAY_URL, options->iDelayUrl != -1);
	EnableDlgItem(hWnd, IDC_DELAY_FILE, options->iDelayFile != -1);
	EnableDlgItem(hWnd, IDC_DELAY_OTHERS, options->iDelayOthers != -1);
}

static INT_PTR CALLBACK OptionsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		//make dialog represent the current options
		bWmNotify = TRUE;
		SendDlgItemMessage(hWnd, IDC_COLBACK_MESSAGE, CPM_SETCOLOUR, 0, options->colBackMsg);
		SendDlgItemMessage(hWnd, IDC_COLTEXT_MESSAGE, CPM_SETCOLOUR, 0, options->colTextMsg);
		SendDlgItemMessage(hWnd, IDC_COLBACK_URL, CPM_SETCOLOUR, 0, options->colBackUrl);
		SendDlgItemMessage(hWnd, IDC_COLTEXT_URL, CPM_SETCOLOUR, 0, options->colTextUrl);
		SendDlgItemMessage(hWnd, IDC_COLBACK_FILE, CPM_SETCOLOUR, 0, options->colBackFile);
		SendDlgItemMessage(hWnd, IDC_COLTEXT_FILE, CPM_SETCOLOUR, 0, options->colTextFile);
		SendDlgItemMessage(hWnd, IDC_COLBACK_OTHERS, CPM_SETCOLOUR, 0, options->colBackOthers);
		SendDlgItemMessage(hWnd, IDC_COLTEXT_OTHERS, CPM_SETCOLOUR, 0, options->colTextOthers);
		SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_MESSAGE, options->bDefaultColorMsg);
		SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_URL, options->bDefaultColorUrl);
		SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_FILE, options->bDefaultColorFile);
		SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_OTHERS, options->bDefaultColorOthers);
		SetCheckBoxState(hWnd, IDC_CHKMENUITEM, options->bMenuitem);
		SetCheckBoxState(hWnd, IDC_CHKDISABLE, options->bDisable);
		SetCheckBoxState(hWnd, IDC_CHKPREVIEW, options->bPreview);
		SetCheckBoxState(hWnd, IDC_CHKMERGEPOPUP, options->bMergePopup);
		SetCheckBoxState(hWnd, IDC_CHKNOTIFY_MESSAGE, options->maskNotify & MASK_MESSAGE);
		SetCheckBoxState(hWnd, IDC_CHKNOTIFY_URL, options->maskNotify & MASK_URL);
		SetCheckBoxState(hWnd, IDC_CHKNOTIFY_FILE, options->maskNotify & MASK_FILE);
		SetCheckBoxState(hWnd, IDC_CHKNOTIFY_OTHER, options->maskNotify & MASK_OTHER);
		SetCheckBoxState(hWnd, IDC_CHKACTL_DISMISS, options->maskActL & MASK_DISMISS);
		SetCheckBoxState(hWnd, IDC_CHKACTL_OPEN, options->maskActL & MASK_OPEN);
		SetCheckBoxState(hWnd, IDC_CHKACTL_REMOVE, options->maskActL & MASK_REMOVE);
		SetCheckBoxState(hWnd, IDC_CHKACTR_DISMISS, options->maskActR & MASK_DISMISS);
		SetCheckBoxState(hWnd, IDC_CHKACTR_OPEN, options->maskActR & MASK_OPEN);
		SetCheckBoxState(hWnd, IDC_CHKACTR_REMOVE, options->maskActR & MASK_REMOVE);
		SetCheckBoxState(hWnd, IDC_CHKACTTE_DISMISS, options->maskActTE & MASK_DISMISS);
		SetCheckBoxState(hWnd, IDC_CHKACTTE_OPEN, options->maskActTE & MASK_OPEN);
		SetCheckBoxState(hWnd, IDC_CHKACTTE_REMOVE, options->maskActTE & MASK_REMOVE);
		SetCheckBoxState(hWnd, IDC_CHKWINDOWCHECK, options->bMsgWindowCheck);
		SetCheckBoxState(hWnd, IDC_CHKREPLYWINDOW, options->bMsgReplyWindow);
		SetCheckBoxState(hWnd, IDC_CHKSHOWDATE, options->bShowDate);
		SetCheckBoxState(hWnd, IDC_CHKSHOWTIME, options->bShowTime);
		SetCheckBoxState(hWnd, IDC_CHKSHOWHEADERS, options->bShowHeaders);
		SetCheckBoxState(hWnd, IDC_RDNEW, !options->bShowON);
		SetCheckBoxState(hWnd, IDC_RDOLD, options->bShowON);
		SetCheckBoxState(hWnd, IDC_CHKHIDESEND, options->bHideSend);
		SetCheckBoxState(hWnd, IDC_SUPRESSRSS, options->bNoRSS);
		SetCheckBoxState(hWnd, IDC_READCHECK, options->bReadCheck);
		SetCheckBoxState(hWnd, IDC_CHKINFINITE_MESSAGE, options->iDelayMsg == -1);
		SetCheckBoxState(hWnd, IDC_CHKINFINITE_URL, options->iDelayUrl == -1);
		SetCheckBoxState(hWnd, IDC_CHKINFINITE_FILE, options->iDelayFile == -1);
		SetCheckBoxState(hWnd, IDC_CHKINFINITE_OTHERS, options->iDelayOthers == -1);
		SetDlgItemInt(hWnd, IDC_DELAY_MESSAGE, options->iDelayMsg != -1 ? options->iDelayMsg : 0, TRUE);
		SetDlgItemInt(hWnd, IDC_DELAY_URL, options->iDelayUrl != -1 ? options->iDelayUrl : 0, TRUE);
		SetDlgItemInt(hWnd, IDC_DELAY_FILE, options->iDelayFile != -1 ? options->iDelayFile : 0, TRUE);
		SetDlgItemInt(hWnd, IDC_DELAY_OTHERS, options->iDelayOthers != -1 ? options->iDelayOthers : 0, TRUE);
		SetDlgItemInt(hWnd, IDC_NUMBERMSG, options->iNumberMsg, FALSE);
		//update items' states 
		UpdateOptionsDlgItemsState(hWnd);
		bWmNotify = FALSE;
		return TRUE;

	case WM_COMMAND:
		if (!bWmNotify) {
			switch (LOWORD(wParam)) {
			case IDC_PREVIEW:
				PopupPreview(options);
				break;
			default:
				//update options
				options->maskNotify = (IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_MESSAGE) ? MASK_MESSAGE : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_URL) ? MASK_URL : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_FILE) ? MASK_FILE : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_OTHER) ? MASK_OTHER : 0);
				options->maskActL = (IsDlgButtonChecked(hWnd, IDC_CHKACTL_DISMISS) ? MASK_DISMISS : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTL_OPEN) ? MASK_OPEN : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTL_REMOVE) ? MASK_REMOVE : 0);
				options->maskActR = (IsDlgButtonChecked(hWnd, IDC_CHKACTR_DISMISS) ? MASK_DISMISS : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTR_OPEN) ? MASK_OPEN : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTR_REMOVE) ? MASK_REMOVE : 0);
				options->maskActTE = (IsDlgButtonChecked(hWnd, IDC_CHKACTTE_DISMISS) ? MASK_DISMISS : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTTE_OPEN) ? MASK_OPEN : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTTE_REMOVE) ? MASK_REMOVE : 0);
				options->bDefaultColorMsg = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_MESSAGE);
				options->bDefaultColorUrl = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_URL);
				options->bDefaultColorFile = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_FILE);
				options->bDefaultColorOthers = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_OTHERS);
				options->bMenuitem = IsDlgButtonChecked(hWnd, IDC_CHKMENUITEM);
				options->bDisable = IsDlgButtonChecked(hWnd, IDC_CHKDISABLE);
				options->bPreview = IsDlgButtonChecked(hWnd, IDC_CHKPREVIEW);
				options->iDelayMsg = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_MESSAGE) ? -1 : (DWORD)GetDlgItemInt(hWnd, IDC_DELAY_MESSAGE, NULL, FALSE);
				options->iDelayUrl = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_URL) ? -1 : (DWORD)GetDlgItemInt(hWnd, IDC_DELAY_URL, NULL, FALSE);
				options->iDelayFile = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_FILE) ? -1 : (DWORD)GetDlgItemInt(hWnd, IDC_DELAY_FILE, NULL, FALSE);
				options->iDelayOthers = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_OTHERS) ? -1 : (DWORD)GetDlgItemInt(hWnd, IDC_DELAY_OTHERS, NULL, FALSE);
				options->bMergePopup = IsDlgButtonChecked(hWnd, IDC_CHKMERGEPOPUP);
				options->bMsgWindowCheck = IsDlgButtonChecked(hWnd, IDC_CHKWINDOWCHECK);
				options->bMsgReplyWindow = IsDlgButtonChecked(hWnd, IDC_CHKREPLYWINDOW);
				options->bShowDate = IsDlgButtonChecked(hWnd, IDC_CHKSHOWDATE);
				options->bShowTime = IsDlgButtonChecked(hWnd, IDC_CHKSHOWTIME);
				options->bShowHeaders = IsDlgButtonChecked(hWnd, IDC_CHKSHOWHEADERS);
				options->bShowON = IsDlgButtonChecked(hWnd, IDC_RDOLD);
				options->bShowON = BST_UNCHECKED == IsDlgButtonChecked(hWnd, IDC_RDNEW);
				options->bHideSend = IsDlgButtonChecked(hWnd, IDC_CHKHIDESEND);
				options->iNumberMsg = GetDlgItemInt(hWnd, IDC_NUMBERMSG, NULL, FALSE);
				options->bNoRSS = IsDlgButtonChecked(hWnd, IDC_SUPRESSRSS);
				options->bReadCheck = IsDlgButtonChecked(hWnd, IDC_READCHECK);
				//update items' states
				UpdateOptionsDlgItemsState(hWnd);
				if (HIWORD(wParam) == CPN_COLOURCHANGED) {
					options->colBackMsg = SendDlgItemMessage(hWnd, IDC_COLBACK_MESSAGE, CPM_GETCOLOUR, 0, 0);
					options->colTextMsg = SendDlgItemMessage(hWnd, IDC_COLTEXT_MESSAGE, CPM_GETCOLOUR, 0, 0);
					options->colBackUrl = SendDlgItemMessage(hWnd, IDC_COLBACK_URL, CPM_GETCOLOUR, 0, 0);
					options->colTextUrl = SendDlgItemMessage(hWnd, IDC_COLTEXT_URL, CPM_GETCOLOUR, 0, 0);
					options->colBackFile = SendDlgItemMessage(hWnd, IDC_COLBACK_FILE, CPM_GETCOLOUR, 0, 0);
					options->colTextFile = SendDlgItemMessage(hWnd, IDC_COLTEXT_FILE, CPM_GETCOLOUR, 0, 0);
					options->colBackOthers = SendDlgItemMessage(hWnd, IDC_COLBACK_OTHERS, CPM_GETCOLOUR, 0, 0);
					options->colTextOthers = SendDlgItemMessage(hWnd, IDC_COLTEXT_OTHERS, CPM_GETCOLOUR, 0, 0);
				}
				//send changes to mi
				MenuitemUpdate(!options->bDisable);
				//enable "Apply" button
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			OptionsWrite();
			break;

		case PSN_RESET:
			OptionsRead();

			//maybe something changed with the mi
			MenuitemUpdate(!options->bDisable);
			break;
		}
		break;
	}
	return FALSE;
}

int OptionsAdd(HINSTANCE hInst, WPARAM addInfo)
{
	if ( ServiceExists(MS_POPUP_ADDPOPUPT)) {
		OPTIONSDIALOGPAGE odp = { 0 };
		odp.hInstance = hInst;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
		odp.pszTitle = LPGEN("Event Notify");
		odp.pszGroup = LPGEN("Popups");
		odp.flags = ODPF_BOLDGROUPS;
		odp.pfnDlgProc = OptionsDlgProc;
		Options_AddPage(addInfo, &odp);
	}

	return 0;
}

int OptionsInit(PLUGIN_OPTIONS* pluginOptions)
{
	options = pluginOptions;
	OptionsRead();
	return 0;
}

int Opt_DisableNEN(BOOL Status)
{
	options->bDisable = Status;
	OptionsWrite(); // JK: really necessary to write everything here ????
	return 0;
}
