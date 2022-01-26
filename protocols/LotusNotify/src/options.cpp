/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"

#include "debug.h"
#include "resource.h"
#include "version.h"
#include "lotusnotes.h"
#include "LotusNotify.h"

//gets default settings from notes.ini file
static void lookupLotusDefaultSettings(HWND hwndDlg)
{
	char tmp[MAXENVVALUE + 1];
	// Get the info from the .ini file
	if (hLotusDll) {
		if (OSGetEnvironmentString1("MailFile", tmp, MAXENVVALUE)) //path to mail file
			SetDlgItemTextA(hwndDlg, IDC_DATABASE, tmp); //and set fields in opt. dialog

		if (OSGetEnvironmentString1("MailServer", tmp, MAXENVVALUE)) { //server name
			decodeServer(tmp);
			SetDlgItemTextA(hwndDlg, IDC_SERVER, tmp);
		}
	}
}

// get variables values stored in db.
void LoadSettings()
{
	settingInterval = (INT)g_plugin.getDword("LNInterval", 15);
	settingInterval1 = (INT)g_plugin.getDword("LNInterval1", 0);
	settingKeepConnection = g_plugin.getByte("LNKeepConnection", 1);

	DBVARIANT dbv;
	if (!g_plugin.getString("LNDatabase", &dbv)) {
		strncpy_s(settingDatabase, _countof(settingDatabase), dbv.pszVal, _countof(settingDatabase));
		db_free(&dbv);
	}
	if (!g_plugin.getString("LNServer", &dbv)) {
		strncpy_s(settingServer, _countof(settingServer), dbv.pszVal, _countof(settingServer));
		db_free(&dbv);
	}
	if (!g_plugin.getString("LNServerSec", &dbv)) {
		strncpy_s(settingServerSec, _countof(settingServerSec), dbv.pszVal, _countof(settingServerSec));
		db_free(&dbv);
	}
	if (!g_plugin.getString("LNPassword", &dbv)) {
		strncpy_s(settingPassword, _countof(settingPassword), dbv.pszVal, _countof(settingPassword));
		db_free(&dbv);
	}
	if (!g_plugin.getString("LNCommand", &dbv)) {
		strncpy_s(settingCommand, _countof(settingCommand), dbv.pszVal, _countof(settingCommand));
		db_free(&dbv);
	}
	if (!g_plugin.getString("LNParameters", &dbv)) {
		strncpy_s(settingParameters, _countof(settingParameters), dbv.pszVal, _countof(settingParameters));
		db_free(&dbv);
	}

	if (!g_plugin.getWString("LNFilterSender", &dbv)) {
		wcsncpy_s(settingFilterSender, dbv.pwszVal, _TRUNCATE);
		db_free(&dbv);
	}
	if (!g_plugin.getWString("LNFilterSubject", &dbv)) {
		wcsncpy_s(settingFilterSubject, dbv.pwszVal, _TRUNCATE);
		db_free(&dbv);
	}
	if (!g_plugin.getWString("LNFilterTo", &dbv)) {
		wcsncpy_s(settingFilterTo, dbv.pwszVal, _TRUNCATE);
		db_free(&dbv);
	}

	settingOnceOnly = g_plugin.getByte("LNOnceOnly", 0);

	settingNonClickedOnly = g_plugin.getByte("LNNonClickedOnly", 1);
	settingShowError = g_plugin.getByte("LNShowError", 1);
	settingSetColours = g_plugin.getByte("LNSetColours", 0);
	settingBgColor = (COLORREF)g_plugin.getDword("LNBgColor", (uint32_t)0xFFFFFF);
	settingFgColor = (COLORREF)g_plugin.getDword("LNFgColor", (uint32_t)0x000000);
	settingNewest = g_plugin.getByte("LNNewest", 0);
	settingEvenNonClicked = g_plugin.getByte("LNEvenNonClicked", 0);
	settingNewestID = (uint32_t)g_plugin.getDword("LNNewestID", 0);
	settingIniAnswer = g_plugin.getByte("LNIniAnswer", 0);
	settingIniCheck = g_plugin.getByte("LNIniCheck", 0);

	for (int i = 0; i < STATUS_COUNT; i++) {
		char buff[128];
		mir_snprintf(buff, "LNStatus%d", i);
		settingStatus[i] = (g_plugin.getByte(buff, 0) == 1);
	}
}

static void SaveSettings(HWND hwndDlg)
{
	char buff[128];
	GetDlgItemTextA(hwndDlg, IDC_SERVER, settingServer, _countof(settingServer));
	g_plugin.setString("LNServer", settingServer);
	g_plugin.setString("LNServerSec", settingServerSec);
	g_plugin.setString("LNPassword", settingPassword);
	g_plugin.setString("LNDatabase", settingDatabase);
	g_plugin.setDword("LNInterval", settingInterval);
	g_plugin.setDword("LNInterval1", settingInterval1);
	g_plugin.setByte("LNKeepConnection", settingKeepConnection);
	g_plugin.setString("LNCommand", settingCommand);
	g_plugin.setString("LNParameters", settingParameters);
	g_plugin.setByte("LNOnceOnly", settingOnceOnly);
	g_plugin.setByte("LNNonClickedOnly", settingNonClickedOnly);
	g_plugin.setByte("LNShowError", settingShowError);
	g_plugin.setByte("LNSetColours", settingSetColours);
	g_plugin.setDword("LNBgColor", (uint32_t)settingBgColor);
	g_plugin.setDword("LNFgColor", (uint32_t)settingFgColor);
	g_plugin.setByte("LNNewest", settingNewest);
	g_plugin.setByte("LNEvenNonClicked", settingEvenNonClicked);
	g_plugin.setByte("LNIniCheck", settingIniCheck);
	g_plugin.setByte("LNIniAnswer", settingIniAnswer);

	for (int i = 0; i < STATUS_COUNT; i++) {
		mir_snprintf(buff, "LNStatus%d", i);
		settingStatus[i] = (ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_STATUS), i) ? TRUE : FALSE);
		g_plugin.setByte(buff, settingStatus[i] ? 1 : 0);
	}

	settingFilterSender[0] = 0;
	for (int i = 0; i < SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_GETCOUNT, 0, 0); i++) {
		wchar_t text[512] = TEXT("");
		SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_GETLBTEXT, (WPARAM)i, (LPARAM)text);
		wcscat_s(settingFilterSender, _countof(settingFilterSender), text);
		wcscat_s(settingFilterSender, _countof(settingFilterSender), TEXT(";"));
	}
	g_plugin.setWString("LNFilterSender", settingFilterSender);

	settingFilterSubject[0] = 0;
	for (int i = 0; i < SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_GETCOUNT, 0, 0); i++) {
		wchar_t text[512] = TEXT("");
		SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_GETLBTEXT, (WPARAM)i, (LPARAM)text);
		wcscat_s(settingFilterSubject, _countof(settingFilterSubject), text);
		wcscat_s(settingFilterSubject, _countof(settingFilterSubject), TEXT(";"));
	}
	g_plugin.setWString("LNFilterSubject", settingFilterSubject);

	settingFilterTo[0] = 0;
	for (int i = 0; i < SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_GETCOUNT, 0, 0); i++) {
		wchar_t text[512] = TEXT("");
		SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_GETLBTEXT, (WPARAM)i, (LPARAM)text);
		wcscat_s(settingFilterTo, _countof(settingFilterTo), text);
		wcscat_s(settingFilterTo, _countof(settingFilterTo), TEXT(";"));
	}
	g_plugin.setWString("LNFilterTo", settingFilterTo);
}

/////////////////////////////////////////////////////////////////////////////////////////
//fill combo in options dlgbox with all known servers

static void fillServersList(HWND hwndDlg)
{
	HANDLE    hServerList = NULLHANDLE;
	uint8_t far *pServerList;            /* Pointer to start of Server List */
	uint16_t      wServerCount;           /* Number of servers in list. */
	uint16_t far *pwServerLength;         /* Index to array of servername lens */
	uint8_t far *pServerName;
	STATUS    error = NOERROR;        /* Error return from API routines. */
	char      ServerString[MAXPATH];  /* String to hold server names.   */
	LPSTR     szServerString = ServerString;

	if (!hLotusDll) {
		return;
	}

	error = NSGetServerList1(nullptr, &hServerList);
	if (error == NOERROR) {

		pServerList = (uint8_t far *) OSLockObject1(hServerList);
		wServerCount = (uint16_t)*pServerList;

		pwServerLength = (uint16_t *)(pServerList + sizeof(uint16_t));

		pServerName = (uint8_t far *) pServerList + sizeof(wServerCount) + ((wServerCount) * sizeof(uint16_t));

		for (USHORT i = 0; i < wServerCount; pServerName += pwServerLength[i], i++) {
			memmove(szServerString, pServerName, pwServerLength[i]);
			szServerString[pwServerLength[i]] = '\0';
			decodeServer(ServerString);
			SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, (LPARAM)szServerString);
		}
		OSUnlockObject1(hServerList);
		OSMemFree1(hServerList);

	}
	else {
		ErMsgByLotusCode(error);
	}
}

//callback function to speak with user interactions in options page
static INT_PTR CALLBACK DlgProcLotusNotifyConnectionOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInit = false;

	switch (msg) {
	case WM_INITDIALOG://initialize dialog, so set properties from db.
		bInit = true;
		TranslateDialogDefault(hwndDlg);//translate miranda function
		LoadSettings();
		CheckDlgButton(hwndDlg, IDC_BUTTON_CHECK, settingIniCheck ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemTextA(hwndDlg, IDC_SERVER, settingServer);
		SetDlgItemTextA(hwndDlg, IDC_SERVERSEC, settingServerSec);
		SetDlgItemTextA(hwndDlg, IDC_DATABASE, settingDatabase);
		SetDlgItemTextA(hwndDlg, IDC_PASSWORD, settingPassword);
		SetDlgItemInt(hwndDlg, IDC_INTERVAL, settingInterval, FALSE);
		CheckDlgButton(hwndDlg, IDC_KEEP_CONNEXION_ON_ERROR, settingKeepConnection ? BST_CHECKED : BST_UNCHECKED);
		bInit = false;
		break;

	case WM_COMMAND://user changed something, so get changes to variables
		if (!bInit) {
			switch (HIWORD(wParam)) {
			case EN_CHANGE:     // text is modified in an edit ctrl
			case BN_CLICKED:    // a checkbox is modified
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_DETECT:
			lookupLotusDefaultSettings(hwndDlg);
			GetDlgItemTextA(hwndDlg, IDC_SERVER, settingServer, _countof(settingServer));
			GetDlgItemTextA(hwndDlg, IDC_DATABASE, settingDatabase, _countof(settingDatabase));
			break;
		case IDC_BUTTON_CHECK:
			settingIniCheck = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_BUTTON_CHECK);
			checkNotesIniFile(TRUE);
			break;
		case IDC_DATABASE:
			GetDlgItemTextA(hwndDlg, IDC_DATABASE, settingDatabase, _countof(settingDatabase));
			break;
		case IDC_SERVER:
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
				{
					int i = SendDlgItemMessage(hwndDlg, IDC_SERVER, CB_GETCURSEL, 0, 0);
					char text[MAXENVVALUE];
					SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_GETLBTEXT, (WPARAM)i, (LPARAM)text);
					SetDlgItemTextA(hwndDlg, IDC_SERVER, text);
					if (!bInit) {
						PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					break;
				}

			case CBN_DROPDOWN:
				SendDlgItemMessage(hwndDlg, IDC_SERVER, CB_RESETCONTENT, 0, 0);
				fillServersList(hwndDlg);
				SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, (LPARAM)settingServer);
				SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_SELECTSTRING, -1, (LPARAM)settingServer);
				break;
			}
			break;
		case IDC_SERVERSEC:
			GetDlgItemTextA(hwndDlg, IDC_SERVERSEC, settingServerSec, _countof(settingServerSec));
			break;
		case IDC_PASSWORD:
			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, settingPassword, _countof(settingPassword));
			break;
		case IDC_INTERVAL:
			settingInterval = GetDlgItemInt(hwndDlg, IDC_INTERVAL, nullptr, FALSE);
			break;
		case IDC_KEEP_CONNEXION_ON_ERROR:
			settingKeepConnection = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_KEEP_CONNEXION_ON_ERROR);
			break;
		}
		break;

	case WM_NOTIFY://apply changes so write it to db
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				LoadSettings();
				return TRUE;

			case PSN_APPLY:
				SaveSettings(hwndDlg);
				return TRUE;
			}
			break;
		} //id from

		break; //switch(msg)

	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcLotusNotifyPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInit = false;

	switch (msg) {
	case WM_INITDIALOG://initialize dialog, so set properties from db.
		bInit = true;
		TranslateDialogDefault(hwndDlg);//translate miranda function
		LoadSettings();

		CheckDlgButton(hwndDlg, IDC_SETCOLOURS, settingSetColours ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, (LPARAM)settingBgColor);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), settingSetColours != 0);
		SendDlgItemMessage(hwndDlg, IDC_FGCOLOR, CPM_SETCOLOUR, 0, (LPARAM)settingFgColor);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FGCOLOR), settingSetColours != 0);

		SetDlgItemInt(hwndDlg, IDC_INTERVAL1, settingInterval1, TRUE);
		CheckDlgButton(hwndDlg, IDC_ONCEONLY, settingOnceOnly ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NONCLICKEDONLY, settingNonClickedOnly ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_NONCLICKEDONLY), settingOnceOnly != 0);
		CheckDlgButton(hwndDlg, IDC_SHOWERROR, settingShowError ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NEWEST, settingNewest ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_REMEMBEREVENNONCLICKED, settingEvenNonClicked ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_REMEMBEREVENNONCLICKED), settingNewest != 0);
		SetDlgItemTextA(hwndDlg, IDC_COMMAND, settingCommand);
		SetDlgItemTextA(hwndDlg, IDC_PARAMETERS, settingParameters);

		bInit = FALSE;
		break;

	case WM_COMMAND://user changed something, so get changes to variables
		if (!bInit) {
			switch (HIWORD(wParam)) {
			case EN_CHANGE:         // text is modified in an edit ctrl
			case BN_CLICKED:        // a checkbox is modified
			case CPN_COLOURCHANGED: // a color has changed
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		switch (LOWORD(wParam)) {
		case IDC_SETCOLOURS:
			settingSetColours = IsDlgButtonChecked(hwndDlg, IDC_SETCOLOURS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), settingSetColours);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FGCOLOR), settingSetColours);
			break;
		case IDC_BGCOLOR:
			settingBgColor = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0);
			break;
		case IDC_FGCOLOR:
			settingFgColor = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_FGCOLOR, CPM_GETCOLOUR, 0, 0);
			break;
		case IDC_INTERVAL1:
			settingInterval1 = GetDlgItemInt(hwndDlg, IDC_INTERVAL1, nullptr, TRUE);
			break;
		case IDC_ONCEONLY:
			settingOnceOnly = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ONCEONLY);
			EnableWindow(GetDlgItem(hwndDlg, IDC_NONCLICKEDONLY), settingOnceOnly);
			break;
		case IDC_NONCLICKEDONLY:
			settingNonClickedOnly = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_NONCLICKEDONLY);
			break;
		case IDC_SHOWERROR:
			settingShowError = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SHOWERROR);
			break;
		case IDC_NEWEST:
			settingNewest = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_NEWEST);
			EnableWindow(GetDlgItem(hwndDlg, IDC_REMEMBEREVENNONCLICKED), settingNewest);
			break;
		case IDC_REMEMBEREVENNONCLICKED:
			settingEvenNonClicked = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_REMEMBEREVENNONCLICKED);
			break;
		case IDC_COMMAND:
			GetDlgItemTextA(hwndDlg, IDC_COMMAND, settingCommand, _countof(settingCommand));
			break;
		case IDC_PARAMETERS:
			GetDlgItemTextA(hwndDlg, IDC_PARAMETERS, settingParameters, _countof(settingParameters));
			break;
		case IDC_BUTTON_CLEAR:
			deleteElements();
			break;
		}
		break;

	case WM_NOTIFY://apply changes so write it to db
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			{
				switch (((LPNMHDR)lParam)->code) {
				case PSN_RESET:
					LoadSettings();
					return TRUE;
				case PSN_APPLY:
					SaveSettings(hwndDlg);

					return TRUE;
					break;
				}
				//KillTimer(hTimerWnd,TID);
				//if(settingInterval!=0)
				//	SetTimer(hTimerWnd, TID, settingInterval*60000, (TIMERPROC)atTime);

				break;
			} //case 0
		} //id from

		break; //switch(msg)

	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcLotusNotifyMiscOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInit = false;

	wchar_t *strptr;
	LVITEM lvI = { 0 };
	LVCOLUMN lvc = { 0 };
	switch (msg) {
	case WM_INITDIALOG://initialize dialog, so set properties from db.
		{
			wchar_t buff[512];
			bInit = true;
			TranslateDialogDefault(hwndDlg);//translate miranda function
			LoadSettings();

			//fill filter combos

			wcsncpy_s(buff, settingFilterSender, _TRUNCATE);
			while (strptr = wcschr(buff, TEXT(';'))) {
				wchar_t tmp[512] = TEXT("");
				wcsncpy_s(tmp, buff, (strptr - buff));
				SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_ADDSTRING, 0, (LPARAM)tmp);
				wcsncpy_s(buff, strptr + 1, _TRUNCATE);
			}

			wcsncpy_s(buff, settingFilterSubject, _TRUNCATE);
			while (strptr = wcschr(buff, TEXT(';'))) {
				wchar_t tmp[512] = TEXT("");
				wcsncpy_s(tmp, buff, (strptr - buff));
				SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_ADDSTRING, 0, (LPARAM)tmp);
				wcsncpy_s(buff, strptr + 1, _TRUNCATE);
			}

			wcsncpy_s(buff, settingFilterTo, _TRUNCATE);
			while (strptr = wcschr(buff, TEXT(';'))) {
				wchar_t tmp[512] = TEXT("");
				wcsncpy_s(tmp, buff, (strptr - buff));
				SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_ADDSTRING, 0, (LPARAM)tmp);
				wcsncpy_s(buff, strptr + 1, _TRUNCATE);
			}

			// initialise and fill listbox
			HWND hwndList = GetDlgItem(hwndDlg, IDC_STATUS);
			ListView_DeleteAllItems(hwndList);

			SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid.
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Status");
			lvc.cx = 120;     // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all items.
			lvI.mask = LVIF_TEXT;
			for (int i = 0; i < STATUS_COUNT; i++) {
				lvI.pszText = Clist_GetStatusModeDescription(ID_STATUS_ONLINE + i, 0);
				lvI.iItem = i;
				ListView_InsertItem(hwndList, &lvI);
				ListView_SetCheckState(hwndList, i, settingStatus[i]);
			}

			bInit = false;
			break;
		}
	case WM_COMMAND://user changed something, so get changes to variables
		{
			if (!bInit && (HIWORD(wParam) == EN_CHANGE)) {
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			char tmp[255];
			int index, size;
			switch (LOWORD(wParam)) {
			case IDC_BUTTON_ADD_SENDER_FILTER:
				GetDlgItemTextA(hwndDlg, IDC_FILTER_SENDER, tmp, _countof(tmp));
				if (strlen(tmp) > 0) {
					SendDlgItemMessageA(hwndDlg, IDC_FILTER_SENDER, CB_ADDSTRING, 0, (LPARAM)tmp);
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			case IDC_BUTTON_REMOVE_SENDER_FILTER:
				index = SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_GETCURSEL, 0, 0);
				size = SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_DELETESTRING, index, 0);
				SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_SETCURSEL, min(index, size - 1), 0);
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_BUTTON_ADD_SUBJECT_FILTER:
				GetDlgItemTextA(hwndDlg, IDC_FILTER_SUBJECT, tmp, _countof(tmp));
				if (strlen(tmp) > 0) {
					SendDlgItemMessageA(hwndDlg, IDC_FILTER_SUBJECT, CB_ADDSTRING, 0, (LPARAM)tmp);
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			case IDC_BUTTON_REMOVE_SUBJECT_FILTER:
				index = SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_GETCURSEL, 0, 0);
				size = SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_DELETESTRING, index, 0);
				SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_SETCURSEL, min(index, size - 1), 0);
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_BUTTON_ADD_TO_FILTER:
				GetDlgItemTextA(hwndDlg, IDC_FILTER_TO, tmp, _countof(tmp));
				if (strlen(tmp) > 0) {
					SendDlgItemMessageA(hwndDlg, IDC_FILTER_TO, CB_ADDSTRING, 0, (LPARAM)tmp);
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			case IDC_BUTTON_REMOVE_TO_FILTER:
				index = SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_GETCURSEL, 0, 0);
				size = SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_DELETESTRING, index, 0);
				SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_SETCURSEL, min(index, size - 1), 0);
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			break;
		}
	case WM_NOTIFY://apply changes so write it to db
		if (bInit) {
			break;
		}
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				LoadSettings();
				return TRUE;

			case PSN_APPLY:
				SaveSettings(hwndDlg);
				return TRUE;
			}

			break;
		} //id from

		if (GetDlgItem(hwndDlg, IDC_STATUS) == ((LPNMHDR)lParam)->hwndFrom) {
			switch (((LPNMHDR)lParam)->code) {
			case LVN_ITEMCHANGED:
				{
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
					if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					break;

				}
				break;
			}
		}
		break; //switch(msg)

	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// options page on miranda called

int LotusNotifyOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Plugins");
	odp.szTitle.w = _A2W(__PLUGIN_NAME);
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;

	odp.szTab.w = LPGENW("Connection");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LOTUS_CONECTION);
	odp.pfnDlgProc = DlgProcLotusNotifyConnectionOpts;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Popup");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LOTUS_POPUP);
	odp.pfnDlgProc = DlgProcLotusNotifyPopupOpts;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Miscellaneous");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LOTUS_MISC);
	odp.pfnDlgProc = DlgProcLotusNotifyMiscOpts;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
