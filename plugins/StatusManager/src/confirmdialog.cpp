/*
	AdvancedAutoAway Plugin for Miranda-IM (www.miranda-im.org)
	KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
	StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
	Copyright 2003-2006 P. Boon

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

#define TIMER_ID	1

// variables
static HWND hwndConfirm;
static int timeOut;

static TProtoSettings *confirmSettings;

static INT_PTR CALLBACK StatusMessageDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PROTOCOLSETTINGEX *protoSetting = nullptr;
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		protoSetting = (PROTOCOLSETTINGEX *)lParam;
		if (protoSetting->m_szMsg == nullptr) {
			wchar_t* smsg = GetDefaultStatusMessage(protoSetting, GetActualStatus(protoSetting));
			if (smsg != nullptr) {
				SetDlgItemText(hwndDlg, IDC_STSMSG, smsg);
				mir_free(smsg);
			}
		}
		else SetDlgItemText(hwndDlg, IDC_STSMSG, protoSetting->m_szMsg);

		wchar_t desc[512];
		mir_snwprintf(desc, TranslateT("Set %s message for %s."),
			Clist_GetStatusModeDescription(GetActualStatus(protoSetting), 0), protoSetting->m_tszAccName);
		SetDlgItemText(hwndDlg, IDC_DESCRIPTION, desc);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_OK:
			{
				int len = SendDlgItemMessage(hwndDlg, IDC_STSMSG, WM_GETTEXTLENGTH, 0, 0);
				if (len > 0) {
					protoSetting->m_szMsg = (wchar_t*)mir_realloc(protoSetting->m_szMsg, sizeof(wchar_t)*(len + 1));
					if (protoSetting->m_szMsg != nullptr)
						GetDlgItemText(hwndDlg, IDC_STSMSG, protoSetting->m_szMsg, len + 1);
				}
			}
			SendMessage(GetParent(hwndDlg), UM_STSMSGDLGCLOSED, TRUE, 0);
			EndDialog(hwndDlg, IDC_OK);
			break;

		case IDC_CANCEL:
			SendMessage(GetParent(hwndDlg), UM_STSMSGDLGCLOSED, 0, 0);
			EndDialog(hwndDlg, IDC_CANCEL);
			break;
		}
		break;
	}

	return FALSE;
}

static int SetStatusList(HWND hwndDlg)
{
	if (confirmSettings->getCount() == 0)
		return -1;

	HWND hList = GetDlgItem(hwndDlg, IDC_STARTUPLIST);
	wchar_t buf[100];

	// create items
	LVITEM lvItem = { 0 };
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;

	for (auto &it : *confirmSettings) {
		lvItem.pszText = it->m_tszAccName;
		if (ListView_GetItemCount(hList) < confirmSettings->getCount())
			ListView_InsertItem(hList, &lvItem);

		int actualStatus;
		switch (it->m_status) {
			case ID_STATUS_LAST:    actualStatus = it->m_lastStatus;   break;
			case ID_STATUS_CURRENT: actualStatus = Proto_GetStatus(it->m_szName); break;
			default:                actualStatus = it->m_status;
		}
		wchar_t *status = Clist_GetStatusModeDescription(actualStatus, 0);
		switch (it->m_status) {
		case ID_STATUS_LAST:
			mir_snwprintf(buf, L"%s (%s)", TranslateT("<last>"), status);
			ListView_SetItemText(hList, lvItem.iItem, 1, buf);
			break;
		case ID_STATUS_CURRENT:
			mir_snwprintf(buf, L"%s (%s)", TranslateT("<current>"), status);
			ListView_SetItemText(hList, lvItem.iItem, 1, buf);
			break;
		default:
			ListView_SetItemText(hList, lvItem.iItem, 1, status);
		}

		// status message
		if (!((!((CallProtoService(it->m_szName, PS_GETCAPS, (WPARAM)PFLAGNUM_1, 0)&PF1_MODEMSGSEND)&~PF1_INDIVMODEMSG)) || (!(CallProtoService(it->m_szName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(actualStatus))))) {
			wchar_t *msg = GetDefaultStatusMessage(it, actualStatus);
			if (msg != nullptr) {
				wchar_t* fMsg = variables_parsedup(msg, it->m_tszAccName, 0);
				ListView_SetItemText(hList, lvItem.iItem, 2, fMsg);
				mir_free(fMsg);
				mir_free(msg);
			}
			else ListView_SetItemText(hList, lvItem.iItem, 2, TranslateT("<n/a>"));
		}
		else ListView_SetItemText(hList, lvItem.iItem, 2, TranslateT("<n/a>"));

		ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hList, 2, LVSCW_AUTOSIZE);
		lvItem.lParam = (LPARAM)it;
		ListView_SetItem(hList, &lvItem);
		lvItem.iItem++;
	}

	// grey out status box
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS), (ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_STARTUPLIST), -1, LVNI_SELECTED) >= 0));
	return 0;
}

static INT_PTR CALLBACK ConfirmDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		hwndConfirm = hwndDlg;
		{
			HWND hList = GetDlgItem(hwndDlg, IDC_STARTUPLIST);
			SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

			// create columns
			LVCOLUMN lvCol = { 0 };
			lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			lvCol.pszText = TranslateT("Protocol");
			ListView_InsertColumn(hList, 0, &lvCol);

			lvCol.cx = 100;
			lvCol.pszText = TranslateT("Status");
			ListView_InsertColumn(hList, 1, &lvCol);

			lvCol.pszText = TranslateT("Message");
			ListView_InsertColumn(hList, 2, &lvCol);
		}

		// create items
		SetStatusList(hwndDlg);

		EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), FALSE);
		// fill profile combo box
		if (!ServiceExists(MS_SS_GETPROFILE))
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), FALSE);
		else {
			int defaultProfile;
			int profileCount = (int)CallService(MS_SS_GETPROFILECOUNT, (WPARAM)&defaultProfile, 0);
			for (int i = 0; i < profileCount; i++) {
				wchar_t profileName[128];
				CallService(MS_SS_GETPROFILENAME, i, (LPARAM)profileName);
				int item = SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_ADDSTRING, 0, (LPARAM)profileName);
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETITEMDATA, item, i);
			}
			if (profileCount == 0)
				EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), FALSE);
			else
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETCURSEL, defaultProfile, 0);
		}

		// start timer
		if (timeOut > 0) {
			wchar_t text[32];
			mir_snwprintf(text, TranslateT("Closing in %d"), timeOut);
			SetDlgItemText(hwndDlg, IDC_CLOSE, text);
			SetTimer(hwndDlg, TIMER_ID, 1000, nullptr);
		}
		return TRUE;

	case WM_TIMER:
		{
			wchar_t text[32];
			mir_snwprintf(text, TranslateT("Closing in %d"), timeOut - 1);
			SetDlgItemText(hwndDlg, IDC_CLOSE, text);
			if (timeOut <= 0) {
				KillTimer(hwndDlg, TIMER_ID);
				SendMessage(hwndDlg, UM_CLOSECONFIRMDLG, 0, 0);
			}
			else timeOut--;
		}
		break;

	case WM_COMMAND:
		// stop timer
		KillTimer(hwndDlg, TIMER_ID);
		SetDlgItemText(hwndDlg, IDC_CLOSE, TranslateT("Close"));
		if ((HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == BN_CLICKED) || (HIWORD(wParam) == NM_CLICK)) { // something clicked
			switch (LOWORD(wParam)) {
			case IDC_PROFILE:
				{
					int profile = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
					for (auto &it : *confirmSettings)
						replaceStrW(it->m_szMsg, nullptr);

					CallService(MS_SS_GETPROFILE, (WPARAM)profile, (LPARAM)confirmSettings);
					for (auto &it : *confirmSettings)
						it->m_szMsg = mir_wstrdup(it->m_szMsg);

					SetStatusList(hwndDlg);
				}
				break;

			case IDC_STATUS:
				{
					LVITEM lvItem = { 0 };
					lvItem.mask = LVIF_TEXT | LVIF_PARAM;
					lvItem.iSubItem = 0;
					lvItem.iItem = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_STARTUPLIST), -1, LVNI_SELECTED);
					if (lvItem.iItem == -1)
						break;

					ListView_GetItem(GetDlgItem(hwndDlg, IDC_STARTUPLIST), &lvItem);
					PROTOCOLSETTINGEX* proto = (PROTOCOLSETTINGEX*)lvItem.lParam;
					int actualStatus = proto->m_status = (int)SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_GETCURSEL, 0, 0), 0);

					// LAST STATUS
					if (proto->m_status == ID_STATUS_LAST) {
						wchar_t last[80];
						mir_snwprintf(last, L"%s (%s)", TranslateT("<last>"), Clist_GetStatusModeDescription(proto->m_lastStatus, 0));
						ListView_SetItemText(GetDlgItem(hwndDlg, IDC_STARTUPLIST), lvItem.iItem, 1, last);
						actualStatus = proto->m_lastStatus;
					}

					// CURRENT STATUS
					else if (proto->m_status == ID_STATUS_CURRENT) {
						int currentStatus = Proto_GetStatus(proto->m_szName);
						wchar_t current[80];
						mir_snwprintf(current, L"%s (%s)", TranslateT("<current>"), Clist_GetStatusModeDescription(currentStatus, 0));
						ListView_SetItemText(GetDlgItem(hwndDlg, IDC_STARTUPLIST), lvItem.iItem, 1, current);
						actualStatus = currentStatus;
					}
					else ListView_SetItemText(GetDlgItem(hwndDlg, IDC_STARTUPLIST), lvItem.iItem, 1, Clist_GetStatusModeDescription(proto->m_status, 0));

					if ((!((CallProtoService(proto->m_szName, PS_GETCAPS, (WPARAM)PFLAGNUM_1, 0)&PF1_MODEMSGSEND)&~PF1_INDIVMODEMSG)) || (!(CallProtoService(proto->m_szName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(actualStatus))))
						EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), FALSE);
					else if ((proto->m_status == ID_STATUS_LAST) || (proto->m_status == ID_STATUS_CURRENT))
						EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), TRUE);
					else
						EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), (CallProtoService(proto->m_szName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(actualStatus)) ? TRUE : FALSE);
					SetStatusList(hwndDlg);
				}
				break;

			case IDC_SETSTSMSG:
				{
					LVITEM lvItem = { 0 };
					lvItem.mask = LVIF_TEXT | LVIF_PARAM;
					lvItem.iSubItem = 0;
					lvItem.iItem = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_STARTUPLIST), -1, LVNI_SELECTED);
					if (lvItem.iItem == -1)
						break;

					ListView_GetItem(GetDlgItem(hwndDlg, IDC_STARTUPLIST), &lvItem);
					DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SETSTSMSGDIALOG), hwndDlg, StatusMessageDlgProc, lvItem.lParam);
				}
				break;

			case IDC_CLOSE:
				SendMessage(hwndDlg, UM_CLOSECONFIRMDLG, 0, 0);
				break;

			case IDC_CANCEL:
				DestroyWindow(hwndDlg);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_STARTUPLIST:
			if (((NMHDR*)lParam)->code == NM_CLICK) {
				KillTimer(hwndDlg, TIMER_ID);
				SetDlgItemText(hwndDlg, IDC_CLOSE, TranslateT("Close"));

				LVITEM lvItem = { 0 };
				lvItem.mask = LVIF_TEXT | LVIF_PARAM;
				lvItem.iSubItem = 0;
				lvItem.iItem = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_STARTUPLIST), -1, LVNI_SELECTED);

				if (ListView_GetItem(GetDlgItem(hwndDlg, IDC_STARTUPLIST), &lvItem) == FALSE) {
					SetStatusList(hwndDlg);
					break;
				}

				PROTOCOLSETTINGEX *proto = (PROTOCOLSETTINGEX*)lvItem.lParam;
				int flags = GetStatusFlags(proto->m_szName);
				// clear box and add new status, loop status and check if compatible with proto
				SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_RESETCONTENT, 0, 0);
				int actualStatus = proto->m_status;

				// last
				wchar_t buf[100];
				mir_snwprintf(buf, L"%s (%s)", TranslateT("<last>"), Clist_GetStatusModeDescription(proto->m_lastStatus, 0));
				int item = SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_ADDSTRING, 0, (LPARAM)buf);
				SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_SETITEMDATA, item, ID_STATUS_LAST);
				if (proto->m_status == ID_STATUS_LAST) {
					SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_SETCURSEL, item, 0);
					actualStatus = proto->m_lastStatus;
				}

				// current
				int currentStatus = Proto_GetStatus(proto->m_szName);
				mir_snwprintf(buf, L"%s (%s)", TranslateT("<current>"), Clist_GetStatusModeDescription(currentStatus, 0));
				item = SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_ADDSTRING, 0, (LPARAM)buf);
				SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_SETITEMDATA, item, ID_STATUS_CURRENT);
				if (proto->m_status == ID_STATUS_CURRENT) {
					SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_SETCURSEL, item, 0);
					actualStatus = currentStatus;
				}

				for (auto &it : statusModes) {
					int pf5 = CallProtoService(proto->m_szName, PS_GETCAPS, PFLAGNUM_5, 0);
					if (((flags & it.iFlag) || it.iFlag == PF2_OFFLINE) && (!(!(flags & Proto_Status2Flag(it.iFlag))) || (pf5 & Proto_Status2Flag(it.iFlag)))) {
						wchar_t *statusMode = Clist_GetStatusModeDescription(it.iStatus, 0);
						item = SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_ADDSTRING, 0, (LPARAM)statusMode);
						SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_SETITEMDATA, item, it.iStatus);
						if (it.iStatus == proto->m_status)
							SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_SETCURSEL, item, 0);
					}
				}

				// enable status box
				EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS), (ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_STARTUPLIST), -1, LVNI_SELECTED) >= 0));
				if ((!((CallProtoService(proto->m_szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) & ~PF1_INDIVMODEMSG)) || (!(CallProtoService(proto->m_szName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(actualStatus))))
					EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), FALSE);
				else if (proto->m_status == ID_STATUS_LAST || proto->m_status == ID_STATUS_CURRENT)
					EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), TRUE);
				else
					EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), (CallProtoService(proto->m_szName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(actualStatus)) ? TRUE : FALSE);
			}
		}
		break;

	case UM_STSMSGDLGCLOSED:
		SetStatusList(hwndDlg);
		break;

	case UM_CLOSECONFIRMDLG:
		SetStatusEx(*confirmSettings);
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		delete confirmSettings; confirmSettings = nullptr;
		hwndConfirm = nullptr;
		break;
	}

	return 0;
}

HWND ShowConfirmDialogEx(TProtoSettings *params, int _timeout)
{
	delete confirmSettings;
	confirmSettings = new TProtoSettings();

	for (auto &it : *params)
		if (it->m_status != ID_STATUS_DISABLED)
			confirmSettings->insert(new SMProto(*it));

	timeOut = _timeout;
	if (timeOut < 0)
		timeOut = DEF_CLOSE_TIME;

	if (hwndConfirm == nullptr) {
		hwndConfirm = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CONFIRMDIALOG), nullptr, ConfirmDlgProc, 0);
		EnableWindow(hwndConfirm, TRUE);
	}

	return hwndConfirm;
}

void ShutdownConfirmDialog()
{
	if (hwndConfirm)
		DestroyWindow(hwndConfirm);
}
