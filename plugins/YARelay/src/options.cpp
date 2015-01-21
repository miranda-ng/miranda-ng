/*
yaRelay.cpp

Yet Another Relay plugin. v.0.0.0.3
This plugin forwards all incoming messages to any contact.

Features:
 - Forwards all messages from any specified contact (or from all contacts)
 - Works only if your status is equals to specified (of set of statuses)
 - Could be specified any template for sent messages
 - Original message could be split up (by size)
 - Could be specified number of split parts to send
 - Incoming message could be marked as 'read' (optional)
 - Outgoing messages could be saved in history (optional)

(c)2005 Anar Ibragimoff (ai91@mail.ru)

*/

#include "stdafx.h"

/**
* Enable/disable some controls (depends on selected check/radio buttons)
*/
static void OptionsFrameEnableControls(HWND hwndDlg)
{
	HWND fromRadioAll = GetDlgItem(hwndDlg, IDC_RADIO_ALL);
	HWND fromCombo = GetDlgItem(hwndDlg, IDC_COMBO_FROM);
	HWND splitChk = GetDlgItem(hwndDlg, IDC_CHECK_SPLIT);
	HWND splitMaxSize = GetDlgItem(hwndDlg, IDC_EDIT_MAXSIZE);
	HWND splitSendParts = GetDlgItem(hwndDlg, IDC_EDIT_SENDPARTS);

	if ((SendMessage(fromRadioAll, BM_GETCHECK, 0, 0) & BST_CHECKED) == 0)
		EnableWindow(fromCombo, TRUE);
	else
		EnableWindow(fromCombo, FALSE);

	if (SendMessage(splitChk, BM_GETCHECK, 0, 0) != BST_CHECKED){
		EnableWindow(splitMaxSize, FALSE);
		EnableWindow(splitSendParts, FALSE);
	}
	else{
		EnableWindow(splitMaxSize, TRUE);
		EnableWindow(splitSendParts, TRUE);
	}
}

/**
* Options panel function
*/
static INT_PTR CALLBACK OptionsFrameProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact;
	int idx;
	switch (uMsg) {
	case WM_INITDIALOG:	
		TranslateDialogDefault(hwndDlg);

		// fill FROM and TO comboboxes
		SendDlgItemMessage(hwndDlg, IDC_COMBO_FROM, CB_RESETCONTENT, 0, 0); 
		SendDlgItemMessage(hwndDlg, IDC_COMBO_TO, CB_RESETCONTENT, 0, 0); 

		idx = SendDlgItemMessage(hwndDlg, IDC_COMBO_FROM, CB_ADDSTRING, 0, (LPARAM)TranslateT("!EVERYONE!")); 
		SendDlgItemMessage(hwndDlg, IDC_COMBO_FROM, CB_SETITEMDATA, (WPARAM)idx, 0);
		SendDlgItemMessage(hwndDlg, IDC_COMBO_FROM, CB_SETCURSEL, (WPARAM)idx, 0);

		idx = SendDlgItemMessage(hwndDlg, IDC_COMBO_TO, CB_ADDSTRING, 0, (LPARAM)TranslateT("!DON'T FORWARD!")); 
		SendDlgItemMessage(hwndDlg, IDC_COMBO_TO, CB_SETITEMDATA, (WPARAM)idx, 0);
		SendDlgItemMessage(hwndDlg, IDC_COMBO_TO, CB_SETCURSEL, (WPARAM)idx, 0);

		for (hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			TCHAR *ptszNick = pcli->pfnGetContactDisplayName(hContact, 0);
			if (ptszNick){
				idx = SendDlgItemMessage(hwndDlg, IDC_COMBO_TO, CB_ADDSTRING, 0, (LPARAM)ptszNick);
				SendDlgItemMessage(hwndDlg, IDC_COMBO_TO, CB_SETITEMDATA, (WPARAM)idx, hContact);

				idx = SendDlgItemMessage(hwndDlg, IDC_COMBO_FROM, CB_ADDSTRING, 0, (LPARAM)ptszNick);
				SendDlgItemMessage(hwndDlg, IDC_COMBO_FROM, CB_SETITEMDATA, (WPARAM)idx, hContact);

				if (hContact == hForwardTo)
					SendDlgItemMessage(hwndDlg, IDC_COMBO_TO, CB_SETCURSEL, (WPARAM)idx, 0);
				if (hContact == hForwardFrom)
					SendDlgItemMessage(hwndDlg, IDC_COMBO_FROM, CB_SETCURSEL, (WPARAM)idx, 0);
			}
		}

		if (hForwardFrom == 0)
			CheckDlgButton(hwndDlg, IDC_RADIO_ALL, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_RADIO_CUSTOM, BST_CHECKED);

		// forward on statuses
		if (iForwardOnStatus & STATUS_OFFLINE  ) CheckDlgButton(hwndDlg, IDC_CHECK1, BST_CHECKED);
		if (iForwardOnStatus & STATUS_ONLINE   ) CheckDlgButton(hwndDlg, IDC_CHECK2, BST_CHECKED);
		if (iForwardOnStatus & STATUS_AWAY     ) CheckDlgButton(hwndDlg, IDC_CHECK3, BST_CHECKED);
		if (iForwardOnStatus & STATUS_NA       ) CheckDlgButton(hwndDlg, IDC_CHECK4, BST_CHECKED);
		if (iForwardOnStatus & STATUS_OCCUPIED ) CheckDlgButton(hwndDlg, IDC_CHECK5, BST_CHECKED);
		if (iForwardOnStatus & STATUS_DND      ) CheckDlgButton(hwndDlg, IDC_CHECK6, BST_CHECKED);
		if (iForwardOnStatus & STATUS_FREECHAT ) CheckDlgButton(hwndDlg, IDC_CHECK7, BST_CHECKED);
		if (iForwardOnStatus & STATUS_INVISIBLE) CheckDlgButton(hwndDlg, IDC_CHECK8, BST_CHECKED);

		// template
		SetDlgItemText(hwndDlg, IDC_EDIT_TEMPLATE, tszForwardTemplate);

		// split
		if (iSplit > 0)
			CheckDlgButton(hwndDlg, IDC_CHECK_SPLIT, BST_CHECKED);

		// max size
		SetDlgItemInt(hwndDlg, IDC_EDIT_MAXSIZE, iSplitMaxSize, FALSE);

		// send parts num
		SetDlgItemInt(hwndDlg, IDC_EDIT_SENDPARTS, iSendParts, FALSE);

		// mark 'read'
		if (iMarkRead > 0)
			CheckDlgButton(hwndDlg, IDC_CHECK_MARKREAD, BST_CHECKED);

		// send and save
		if (iSendAndHistory > 0)
			CheckDlgButton(hwndDlg, IDC_CHECK_SAVEHISTORY, BST_CHECKED);

		// enable/disable controls
		OptionsFrameEnableControls(hwndDlg);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED){
			SendMessage(GetParent(hwndDlg),PSM_CHANGED,0,0);
			switch(LOWORD(wParam)){
			case IDC_RADIO_ALL:
			case IDC_RADIO_CUSTOM:
			case IDC_CHECK_SPLIT:
				OptionsFrameEnableControls(hwndDlg);
				break;
			}
		}
		break;
	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code) {
			case PSN_APPLY:
			case PSN_KILLACTIVE:

				// read all data from options frame
				if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_ALL) == BST_CHECKED)
					hForwardFrom = 0;
				else
					hForwardFrom = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_COMBO_FROM, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_COMBO_FROM, CB_GETCURSEL, 0, 0), 0);

				hForwardTo = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_COMBO_TO, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_COMBO_TO, CB_GETCURSEL, 0, 0), 0);
				iForwardOnStatus = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK1) == BST_CHECKED) iForwardOnStatus |= STATUS_OFFLINE;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK2) == BST_CHECKED) iForwardOnStatus |= STATUS_ONLINE;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK3) == BST_CHECKED) iForwardOnStatus |= STATUS_AWAY;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK4) == BST_CHECKED) iForwardOnStatus |= STATUS_NA;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK5) == BST_CHECKED) iForwardOnStatus |= STATUS_OCCUPIED;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK6) == BST_CHECKED) iForwardOnStatus |= STATUS_DND;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK7) == BST_CHECKED) iForwardOnStatus |= STATUS_FREECHAT;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK8) == BST_CHECKED) iForwardOnStatus |= STATUS_INVISIBLE;
				GetDlgItemText(hwndDlg, IDC_EDIT_TEMPLATE, tszForwardTemplate, SIZEOF(tszForwardTemplate));
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SPLIT) == BST_CHECKED) iSplit = 1; else iSplit = 0;
				iSplitMaxSize = GetDlgItemInt(hwndDlg, IDC_EDIT_MAXSIZE, NULL, FALSE);
				iSendParts = GetDlgItemInt(hwndDlg, IDC_EDIT_SENDPARTS, NULL, FALSE);
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MARKREAD) == BST_CHECKED) iMarkRead = 1; else iMarkRead = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SAVEHISTORY) == BST_CHECKED) iSendAndHistory = 1; else iSendAndHistory = 0;
				if (iSplitMaxSize <= 0)
					iSplitMaxSize = 1;

				// write to database
				db_set_dw(NULL, "yaRelay", "ForwardFrom", (DWORD)hForwardFrom);
				db_set_dw(NULL, "yaRelay", "ForwardTo", (DWORD)hForwardTo);
				db_set_dw(NULL, "yaRelay", "ForwardOnStatus", iForwardOnStatus);
				db_set_ts(NULL, "yaRelay", "ForwardTemplate", tszForwardTemplate);
				db_set_dw(NULL, "yaRelay", "Split", iSplit);
				db_set_dw(NULL, "yaRelay", "SplitMaxSize", iSplitMaxSize);
				db_set_dw(NULL, "yaRelay", "SendParts", iSendParts);
				db_set_dw(NULL, "yaRelay", "MarkRead", iMarkRead);
				db_set_dw(NULL, "yaRelay", "SendAndHistory", iSendAndHistory);
				return TRUE;
			}			
			break;
		}
	}
	return FALSE;
}

/**
* Init options panel
*/
int OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.position = -1;
	odp.pszGroup = LPGEN("Message Sessions");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SETTINGS);
	odp.pszTitle = LPGEN("Messages forwarding");
	odp.pfnDlgProc = OptionsFrameProc;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
