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

#include "stdafx.h"

BOOL bWmNotify;

void CMPlugin::OptionsRead(void)
{
	bDisable = getByte(OPT_DISABLE, FALSE);
	bPreview = getByte(OPT_PREVIEW, TRUE);
	bMenuitem = getByte(OPT_MENUITEM, FALSE);
	bDefaultColorMsg = getByte(OPT_COLDEFAULT_MESSAGE, FALSE);
	bDefaultColorFile = getByte(OPT_COLDEFAULT_FILE, FALSE);
	bDefaultColorOthers = getByte(OPT_COLDEFAULT_OTHERS, FALSE);
	colBackMsg = getDword(OPT_COLBACK_MESSAGE, DEFAULT_COLBACK);
	colTextMsg = getDword(OPT_COLTEXT_MESSAGE, DEFAULT_COLTEXT);
	colBackFile = getDword(OPT_COLBACK_FILE, DEFAULT_COLBACK);
	colTextFile = getDword(OPT_COLTEXT_FILE, DEFAULT_COLTEXT);
	colBackOthers = getDword(OPT_COLBACK_OTHERS, DEFAULT_COLBACK);
	colTextOthers = getDword(OPT_COLTEXT_OTHERS, DEFAULT_COLTEXT);
	maskNotify = getByte(OPT_MASKNOTIFY, DEFAULT_MASKNOTIFY);
	maskActL = getByte(OPT_MASKACTL, DEFAULT_MASKACTL);
	maskActR = getByte(OPT_MASKACTR, DEFAULT_MASKACTR);
	maskActTE = getByte(OPT_MASKACTTE, DEFAULT_MASKACTE);
	bMsgWindowCheck = getByte(OPT_MSGWINDOWCHECK, TRUE);
	bMsgReplyWindow = getByte(OPT_MSGREPLYWINDOW, FALSE);
	bMergePopup = getByte(OPT_MERGEPOPUP, TRUE);
	iDelayMsg = getDword(OPT_DELAY_MESSAGE, DEFAULT_DELAY);
	iDelayFile = getDword(OPT_DELAY_FILE, DEFAULT_DELAY);
	iDelayOthers = getDword(OPT_DELAY_OTHERS, DEFAULT_DELAY);
	iDelayDefault = DBGetContactSettingRangedWord(NULL, "Popup", "Seconds", SETTING_LIFETIME_DEFAULT, SETTING_LIFETIME_MIN, SETTING_LIFETIME_MAX);
	bShowDate = getByte(OPT_SHOW_DATE, TRUE);
	bShowTime = getByte(OPT_SHOW_TIME, TRUE);
	bShowHeaders = getByte(OPT_SHOW_HEADERS, TRUE);
	iNumberMsg = getByte(OPT_NUMBER_MSG, TRUE);
	bShowON = getByte(OPT_SHOW_ON, TRUE);
	bHideSend = getByte(OPT_HIDESEND, TRUE);
	bNoRSS = getByte(OPT_NORSS, FALSE);
	bReadCheck = getByte(OPT_READCHECK, FALSE);
}

void CMPlugin::OptionsWrite(void)
{
	setByte(OPT_DISABLE, bDisable);
	setByte(OPT_PREVIEW, bPreview);
	setByte(OPT_MENUITEM, bMenuitem);
	setByte(OPT_COLDEFAULT_MESSAGE, bDefaultColorMsg);
	setByte(OPT_COLDEFAULT_FILE, bDefaultColorFile);
	setByte(OPT_COLDEFAULT_OTHERS, bDefaultColorOthers);
	setDword(OPT_COLBACK_MESSAGE, colBackMsg);
	setDword(OPT_COLTEXT_MESSAGE, colTextMsg);
	setDword(OPT_COLBACK_FILE, colBackFile);
	setDword(OPT_COLTEXT_FILE, colTextFile);
	setDword(OPT_COLBACK_OTHERS, colBackOthers);
	setDword(OPT_COLTEXT_OTHERS, colTextOthers);
	setByte(OPT_MASKNOTIFY, (uint8_t)maskNotify);
	setByte(OPT_MASKACTL, (uint8_t)maskActL);
	setByte(OPT_MASKACTR, (uint8_t)maskActR);
	setByte(OPT_MASKACTTE, (uint8_t)maskActTE);
	setByte(OPT_MSGWINDOWCHECK, bMsgWindowCheck);
	setByte(OPT_MSGREPLYWINDOW, bMsgReplyWindow);
	setByte(OPT_MERGEPOPUP, bMergePopup);
	setDword(OPT_DELAY_MESSAGE, iDelayMsg);
	setDword(OPT_DELAY_FILE, iDelayFile);
	setDword(OPT_DELAY_OTHERS, iDelayOthers);
	setByte(OPT_SHOW_DATE, bShowDate);
	setByte(OPT_SHOW_TIME, bShowTime);
	setByte(OPT_SHOW_HEADERS, bShowHeaders);
	setByte(OPT_NUMBER_MSG, (uint8_t)iNumberMsg);
	setByte(OPT_SHOW_ON, bShowON);
	setByte(OPT_HIDESEND, bHideSend);
	setByte(OPT_NORSS, bNoRSS);
	setByte(OPT_READCHECK, bReadCheck);
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
	EnableDlgItem(hWnd, IDC_COLBACK_MESSAGE, !g_plugin.bDefaultColorMsg);
	EnableDlgItem(hWnd, IDC_COLTEXT_MESSAGE, !g_plugin.bDefaultColorMsg);
	EnableDlgItem(hWnd, IDC_COLBACK_FILE, !g_plugin.bDefaultColorFile);
	EnableDlgItem(hWnd, IDC_COLTEXT_FILE, !g_plugin.bDefaultColorFile);
	EnableDlgItem(hWnd, IDC_COLBACK_OTHERS, !g_plugin.bDefaultColorOthers);
	EnableDlgItem(hWnd, IDC_COLTEXT_OTHERS, !g_plugin.bDefaultColorOthers);
	//disable merge messages options when is not using
	EnableDlgItem(hWnd, IDC_CHKSHOWDATE, g_plugin.bMergePopup);
	EnableDlgItem(hWnd, IDC_CHKSHOWTIME, g_plugin.bMergePopup);
	EnableDlgItem(hWnd, IDC_CHKSHOWHEADERS, g_plugin.bMergePopup);
	EnableDlgItem(hWnd, IDC_CMDEDITHEADERS, g_plugin.bMergePopup && g_plugin.bShowHeaders);
	EnableDlgItem(hWnd, IDC_NUMBERMSG, g_plugin.bMergePopup);
	EnableDlgItem(hWnd, IDC_LBNUMBERMSG, g_plugin.bMergePopup);
	EnableDlgItem(hWnd, IDC_RDNEW, g_plugin.bMergePopup && g_plugin.iNumberMsg);
	EnableDlgItem(hWnd, IDC_RDOLD, g_plugin.bMergePopup && g_plugin.iNumberMsg);
	//disable delay textbox when infinite is checked
	EnableDlgItem(hWnd, IDC_DELAY_MESSAGE, g_plugin.iDelayMsg != -1);
	EnableDlgItem(hWnd, IDC_DELAY_FILE, g_plugin.iDelayFile != -1);
	EnableDlgItem(hWnd, IDC_DELAY_OTHERS, g_plugin.iDelayOthers != -1);
}

static INT_PTR CALLBACK OptionsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		//make dialog represent the current options
		bWmNotify = TRUE;
		SendDlgItemMessage(hWnd, IDC_COLBACK_MESSAGE, CPM_SETCOLOUR, 0, g_plugin.colBackMsg);
		SendDlgItemMessage(hWnd, IDC_COLTEXT_MESSAGE, CPM_SETCOLOUR, 0, g_plugin.colTextMsg);
		SendDlgItemMessage(hWnd, IDC_COLBACK_FILE, CPM_SETCOLOUR, 0, g_plugin.colBackFile);
		SendDlgItemMessage(hWnd, IDC_COLTEXT_FILE, CPM_SETCOLOUR, 0, g_plugin.colTextFile);
		SendDlgItemMessage(hWnd, IDC_COLBACK_OTHERS, CPM_SETCOLOUR, 0, g_plugin.colBackOthers);
		SendDlgItemMessage(hWnd, IDC_COLTEXT_OTHERS, CPM_SETCOLOUR, 0, g_plugin.colTextOthers);
		SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_MESSAGE, g_plugin.bDefaultColorMsg);
		SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_FILE, g_plugin.bDefaultColorFile);
		SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_OTHERS, g_plugin.bDefaultColorOthers);
		SetCheckBoxState(hWnd, IDC_CHKMENUITEM, g_plugin.bMenuitem);
		SetCheckBoxState(hWnd, IDC_CHKDISABLE, g_plugin.bDisable);
		SetCheckBoxState(hWnd, IDC_CHKPREVIEW, g_plugin.bPreview);
		SetCheckBoxState(hWnd, IDC_CHKMERGEPOPUP, g_plugin.bMergePopup);
		SetCheckBoxState(hWnd, IDC_CHKNOTIFY_MESSAGE, g_plugin.maskNotify & MASK_MESSAGE);
		SetCheckBoxState(hWnd, IDC_CHKNOTIFY_FILE, g_plugin.maskNotify & MASK_FILE);
		SetCheckBoxState(hWnd, IDC_CHKNOTIFY_OTHER, g_plugin.maskNotify & MASK_OTHER);
		SetCheckBoxState(hWnd, IDC_CHKACTL_DISMISS, g_plugin.maskActL & MASK_DISMISS);
		SetCheckBoxState(hWnd, IDC_CHKACTL_OPEN, g_plugin.maskActL & MASK_OPEN);
		SetCheckBoxState(hWnd, IDC_CHKACTL_REMOVE, g_plugin.maskActL & MASK_REMOVE);
		SetCheckBoxState(hWnd, IDC_CHKACTR_DISMISS, g_plugin.maskActR & MASK_DISMISS);
		SetCheckBoxState(hWnd, IDC_CHKACTR_OPEN, g_plugin.maskActR & MASK_OPEN);
		SetCheckBoxState(hWnd, IDC_CHKACTR_REMOVE, g_plugin.maskActR & MASK_REMOVE);
		SetCheckBoxState(hWnd, IDC_CHKACTTE_DISMISS, g_plugin.maskActTE & MASK_DISMISS);
		SetCheckBoxState(hWnd, IDC_CHKACTTE_OPEN, g_plugin.maskActTE & MASK_OPEN);
		SetCheckBoxState(hWnd, IDC_CHKACTTE_REMOVE, g_plugin.maskActTE & MASK_REMOVE);
		SetCheckBoxState(hWnd, IDC_CHKWINDOWCHECK, g_plugin.bMsgWindowCheck);
		SetCheckBoxState(hWnd, IDC_CHKREPLYWINDOW, g_plugin.bMsgReplyWindow);
		SetCheckBoxState(hWnd, IDC_CHKSHOWDATE, g_plugin.bShowDate);
		SetCheckBoxState(hWnd, IDC_CHKSHOWTIME, g_plugin.bShowTime);
		SetCheckBoxState(hWnd, IDC_CHKSHOWHEADERS, g_plugin.bShowHeaders);
		SetCheckBoxState(hWnd, IDC_RDNEW, !g_plugin.bShowON);
		SetCheckBoxState(hWnd, IDC_RDOLD, g_plugin.bShowON);
		SetCheckBoxState(hWnd, IDC_CHKHIDESEND, g_plugin.bHideSend);
		SetCheckBoxState(hWnd, IDC_SUPRESSRSS, g_plugin.bNoRSS);
		SetCheckBoxState(hWnd, IDC_READCHECK, g_plugin.bReadCheck);
		SetCheckBoxState(hWnd, IDC_CHKINFINITE_MESSAGE, g_plugin.iDelayMsg == -1);
		SetCheckBoxState(hWnd, IDC_CHKINFINITE_FILE, g_plugin.iDelayFile == -1);
		SetCheckBoxState(hWnd, IDC_CHKINFINITE_OTHERS, g_plugin.iDelayOthers == -1);
		SetDlgItemInt(hWnd, IDC_DELAY_MESSAGE, g_plugin.iDelayMsg != -1 ? g_plugin.iDelayMsg : 0, TRUE);
		SetDlgItemInt(hWnd, IDC_DELAY_FILE, g_plugin.iDelayFile != -1 ? g_plugin.iDelayFile : 0, TRUE);
		SetDlgItemInt(hWnd, IDC_DELAY_OTHERS, g_plugin.iDelayOthers != -1 ? g_plugin.iDelayOthers : 0, TRUE);
		SetDlgItemInt(hWnd, IDC_NUMBERMSG, g_plugin.iNumberMsg, FALSE);
		//update items' states 
		UpdateOptionsDlgItemsState(hWnd);
		bWmNotify = FALSE;
		return TRUE;

	case WM_COMMAND:
		if (!bWmNotify) {
			switch (LOWORD(wParam)) {
			case IDC_PREVIEW:
				PopupPreview();
				break;
			default:
				//update options
				g_plugin.maskNotify = (IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_MESSAGE) ? MASK_MESSAGE : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_FILE) ? MASK_FILE : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_OTHER) ? MASK_OTHER : 0);
				g_plugin.maskActL = (IsDlgButtonChecked(hWnd, IDC_CHKACTL_DISMISS) ? MASK_DISMISS : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTL_OPEN) ? MASK_OPEN : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTL_REMOVE) ? MASK_REMOVE : 0);
				g_plugin.maskActR = (IsDlgButtonChecked(hWnd, IDC_CHKACTR_DISMISS) ? MASK_DISMISS : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTR_OPEN) ? MASK_OPEN : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTR_REMOVE) ? MASK_REMOVE : 0);
				g_plugin.maskActTE = (IsDlgButtonChecked(hWnd, IDC_CHKACTTE_DISMISS) ? MASK_DISMISS : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTTE_OPEN) ? MASK_OPEN : 0) |
					(IsDlgButtonChecked(hWnd, IDC_CHKACTTE_REMOVE) ? MASK_REMOVE : 0);
				g_plugin.bDefaultColorMsg = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_MESSAGE);
				g_plugin.bDefaultColorFile = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_FILE);
				g_plugin.bDefaultColorOthers = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_OTHERS);
				g_plugin.bMenuitem = IsDlgButtonChecked(hWnd, IDC_CHKMENUITEM);
				g_plugin.bDisable = IsDlgButtonChecked(hWnd, IDC_CHKDISABLE);
				g_plugin.bPreview = IsDlgButtonChecked(hWnd, IDC_CHKPREVIEW);
				g_plugin.iDelayMsg = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_MESSAGE) ? -1 : (uint32_t)GetDlgItemInt(hWnd, IDC_DELAY_MESSAGE, nullptr, FALSE);
				g_plugin.iDelayFile = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_FILE) ? -1 : (uint32_t)GetDlgItemInt(hWnd, IDC_DELAY_FILE, nullptr, FALSE);
				g_plugin.iDelayOthers = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_OTHERS) ? -1 : (uint32_t)GetDlgItemInt(hWnd, IDC_DELAY_OTHERS, nullptr, FALSE);
				g_plugin.bMergePopup = IsDlgButtonChecked(hWnd, IDC_CHKMERGEPOPUP);
				g_plugin.bMsgWindowCheck = IsDlgButtonChecked(hWnd, IDC_CHKWINDOWCHECK);
				g_plugin.bMsgReplyWindow = IsDlgButtonChecked(hWnd, IDC_CHKREPLYWINDOW);
				g_plugin.bShowDate = IsDlgButtonChecked(hWnd, IDC_CHKSHOWDATE);
				g_plugin.bShowTime = IsDlgButtonChecked(hWnd, IDC_CHKSHOWTIME);
				g_plugin.bShowHeaders = IsDlgButtonChecked(hWnd, IDC_CHKSHOWHEADERS);
				g_plugin.bShowON = IsDlgButtonChecked(hWnd, IDC_RDOLD);
				g_plugin.bShowON = BST_UNCHECKED == IsDlgButtonChecked(hWnd, IDC_RDNEW);
				g_plugin.bHideSend = IsDlgButtonChecked(hWnd, IDC_CHKHIDESEND);
				g_plugin.iNumberMsg = GetDlgItemInt(hWnd, IDC_NUMBERMSG, nullptr, FALSE);
				g_plugin.bNoRSS = IsDlgButtonChecked(hWnd, IDC_SUPRESSRSS);
				g_plugin.bReadCheck = IsDlgButtonChecked(hWnd, IDC_READCHECK);
				//update items' states
				UpdateOptionsDlgItemsState(hWnd);
				if (HIWORD(wParam) == CPN_COLOURCHANGED) {
					g_plugin.colBackMsg = SendDlgItemMessage(hWnd, IDC_COLBACK_MESSAGE, CPM_GETCOLOUR, 0, 0);
					g_plugin.colTextMsg = SendDlgItemMessage(hWnd, IDC_COLTEXT_MESSAGE, CPM_GETCOLOUR, 0, 0);
					g_plugin.colBackFile = SendDlgItemMessage(hWnd, IDC_COLBACK_FILE, CPM_GETCOLOUR, 0, 0);
					g_plugin.colTextFile = SendDlgItemMessage(hWnd, IDC_COLTEXT_FILE, CPM_GETCOLOUR, 0, 0);
					g_plugin.colBackOthers = SendDlgItemMessage(hWnd, IDC_COLBACK_OTHERS, CPM_GETCOLOUR, 0, 0);
					g_plugin.colTextOthers = SendDlgItemMessage(hWnd, IDC_COLTEXT_OTHERS, CPM_GETCOLOUR, 0, 0);
				}
				//send changes to mi
				MenuitemUpdate(!g_plugin.bDisable);
				//enable "Apply" button
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			g_plugin.OptionsWrite();
			break;

		case PSN_RESET:
			g_plugin.OptionsRead();

			//maybe something changed with the mi
			MenuitemUpdate(!g_plugin.bDisable);
			break;
		}
		break;
	}
	return FALSE;
}

int OptionsAdd(WPARAM addInfo, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
	odp.szTitle.a = LPGEN("Event Notify");
	odp.szGroup.a = LPGEN("Popups");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = OptionsDlgProc;
	g_plugin.addOptions(addInfo, &odp);
	return 0;
}

int Opt_DisableNEN(BOOL Status)
{
	g_plugin.bDisable = Status;
	g_plugin.OptionsWrite(); // JK: really necessary to write everything here ????
	return 0;
}
