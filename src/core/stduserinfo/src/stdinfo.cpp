/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"

INT_PTR CALLBACK ContactDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define SVS_NORMAL        0
#define SVS_GENDER        1
#define SVS_ZEROISUNSPEC  2
#define SVS_IP            3
#define SVS_COUNTRY       4
#define SVS_MONTH         5
#define SVS_SIGNED        6
#define SVS_TIMEZONE      7
#define SVS_MARITAL    	  8

static void SetValue(HWND hwndDlg, int idCtrl, MCONTACT hContact, char *szModule, char *szSetting, int special)
{
	char *pstr = nullptr;
	wchar_t *pwstr = nullptr, wstr[80];

	DBVARIANT dbv = { DBVT_DELETED };

	int unspecified;
	if (szModule == nullptr)
		unspecified = 1;
	else
		unspecified = db_get_s(hContact, szModule, szSetting, &dbv, 0);

	if (!unspecified) {
		switch (dbv.type) {
		case DBVT_BYTE:
			if (special == SVS_GENDER) {
				if (dbv.cVal == 'M') pwstr = TranslateT("Male");
				else if (dbv.cVal == 'F') pwstr = TranslateT("Female");
				else unspecified = 1;
			}
			else if (special == SVS_MONTH) {
				if (dbv.bVal > 0 && dbv.bVal <= 12) {
					pwstr = wstr;
					GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SABBREVMONTHNAME1 - 1 + dbv.bVal, wstr, _countof(wstr));
				}
				else unspecified = 1;
			}
			else if (special == SVS_TIMEZONE) {
				if (dbv.cVal == -100) unspecified = 1;
				else {
					pwstr = wstr;
					mir_snwprintf(wstr, dbv.cVal ? L"UTC%+d:%02d" : L"UTC", -dbv.cVal / 2, (dbv.cVal & 1) * 30);
				}
			}
			else if (special == SVS_MARITAL) {
				switch (dbv.cVal) {
				case 0:
					pwstr = TranslateT("<not specified>");
					break;
				case 10:
					pwstr = TranslateT("Single");
					break;
				case 11:
					pwstr = TranslateT("Close relationships");
					break;
				case 12:
					pwstr = TranslateT("Engaged");
					break;
				case 20:
					pwstr = TranslateT("Married");
					break;
				case 30:
					pwstr = TranslateT("Divorced");
					break;
				case 31:
					pwstr = TranslateT("Separated");
					break;
				case 40:
					pwstr = TranslateT("Widowed");
					break;
				case 50:
					pwstr = TranslateT("Actively searching");
					break;
				case 60:
					pwstr = TranslateT("In love");
					break;
				case 70:
					pwstr = TranslateT("It's complicated");
					break;
				case 80:
					pwstr = TranslateT("In a civil union");
					break;
				default:
					unspecified = 1;
				}
			}
			else {
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.bVal == 0);
				pwstr = _itow(special == SVS_SIGNED ? dbv.cVal : dbv.bVal, wstr, 10);
			}
			break;

		case DBVT_WORD:
			if (special == SVS_COUNTRY) {
				uint16_t wSave = dbv.wVal;
				if (wSave == (uint16_t)-1) {
					char szSettingName[100];
					mir_snprintf(szSettingName, "%sName", szSetting);
					if (!db_get_ws(hContact, szModule, szSettingName, &dbv)) {
						pwstr = dbv.pwszVal;
						unspecified = false;
						break;
					}
				}

				pwstr = TranslateW(_A2T((char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, wSave, 0)));
				unspecified = pwstr == nullptr;
			}
			else {
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.wVal == 0);
				pwstr = _itow(special == SVS_SIGNED ? dbv.sVal : dbv.wVal, wstr, 10);
			}
			break;

		case DBVT_DWORD:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.dVal == 0);
			if (special == SVS_IP) {
				struct in_addr ia;
				ia.S_un.S_addr = htonl(dbv.dVal);
				mir_wstrncpy(wstr, _A2T(inet_ntoa(ia)), _countof(wstr));
				pwstr = wstr;
				if (dbv.dVal == 0)
					unspecified = 1;
			}
			else pwstr = _itow(special == SVS_SIGNED ? dbv.lVal : dbv.dVal, wstr, 10);
			break;

		case DBVT_ASCIIZ:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
			pstr = dbv.pszVal;
			break;

		case DBVT_UTF8:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
			if (!unspecified) {
				SetDlgItemTextW(hwndDlg, idCtrl, TranslateW(ptrW(mir_utf8decodeW(dbv.pszVal))));
				goto LBL_Exit;
			}

			pstr = dbv.pszVal;
			mir_utf8decode(dbv.pszVal, nullptr);
			break;

		default:
			pwstr = wstr;
			mir_wstrcpy(wstr, L"???");
			break;
		}
	}

	if (unspecified)
		SetDlgItemText(hwndDlg, idCtrl, TranslateT("<not specified>"));
	else if (pwstr != nullptr)
		SetDlgItemText(hwndDlg, idCtrl, pwstr);
	else
		SetDlgItemTextA(hwndDlg, idCtrl, pstr);

LBL_Exit:
	EnableWindow(GetDlgItem(hwndDlg, idCtrl), !unspecified);
	db_free(&dbv);
}

static INT_PTR CALLBACK SummaryDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_INFOCHANGED) {
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				if (hContact != NULL) {
					char *szProto = Proto_GetBaseAccountName(hContact);
					if (szProto == nullptr)
						break;

					SetValue(hwndDlg, IDC_NICK, hContact, szProto, "Nick", 0);
					SetValue(hwndDlg, IDC_FIRSTNAME, hContact, szProto, "FirstName", 0);
					SetValue(hwndDlg, IDC_LASTNAME, hContact, szProto, "LastName", 0);
					SetValue(hwndDlg, IDC_EMAIL, hContact, szProto, "e-mail", 0);
					SetValue(hwndDlg, IDC_AGE, hContact, szProto, "Age", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_GENDER, hContact, szProto, "Gender", SVS_GENDER);
					SetValue(hwndDlg, IDC_DOBDAY, hContact, szProto, "BirthDay", 0);
					SetValue(hwndDlg, IDC_DOBMONTH, hContact, szProto, "BirthMonth", SVS_MONTH);
					SetValue(hwndDlg, IDC_DOBYEAR, hContact, szProto, "BirthYear", 0);
					SetValue(hwndDlg, IDC_MARITAL, hContact, szProto, "MaritalStatus", SVS_MARITAL);
				}
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg), msg, wParam, lParam);
			break;
		case IDC_EMAIL:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EMAIL))) {
				wchar_t szExec[264], szEmail[256];
				GetDlgItemText(hwndDlg, IDC_EMAIL, szEmail, _countof(szEmail));
				mir_snwprintf(szExec, L"mailto:%s", szEmail);
				ShellExecute(hwndDlg, L"open", szExec, NULL, NULL, SW_SHOW);
			}
			break;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK LocationDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		TranslateDialogDefault(hwndDlg);
		SetTimer(hwndDlg, 1, 1000, nullptr);

		TimeZone_PrepareList(lParam, NULL, GetDlgItem(hwndDlg, IDC_TIMEZONESELECT), TZF_PLF_CB);
		SendMessage(hwndDlg, WM_TIMER, 0, 0);
		break;

	case WM_TIMER:
		{
			MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (hContact != NULL) {
				wchar_t szTime[80];
				if (printDateTimeByContact(hContact, L"s", szTime, _countof(szTime), TZF_KNOWNONLY)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_LOCALTIME), FALSE);
					SetDlgItemText(hwndDlg, IDC_LOCALTIME, TranslateT("<not specified>"));
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_LOCALTIME), TRUE);
					SetDlgItemText(hwndDlg, IDC_LOCALTIME, szTime);
				}
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_INFOCHANGED) {
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				if (hContact != NULL) {
					char *szProto = Proto_GetBaseAccountName(hContact);
					if (szProto == nullptr)
						break;

					SetValue(hwndDlg, IDC_STREET, hContact, szProto, "Street", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_CITY, hContact, szProto, "City", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_STATE, hContact, szProto, "State", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_ZIP, hContact, szProto, "ZIP", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_COUNTRY, hContact, szProto, "Country", SVS_COUNTRY);
					SetValue(hwndDlg, IDC_LANGUAGE1, hContact, szProto, "Language1", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_LANGUAGE2, hContact, szProto, "Language2", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_LANGUAGE3, hContact, szProto, "Language3", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_TIMEZONE, hContact, szProto, "Timezone", SVS_TIMEZONE);
				}
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg), msg, wParam, lParam);
			break;

		case IDC_TIMEZONESELECT:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				TimeZone_StoreListResult(hContact, NULL, GetDlgItem(hwndDlg, IDC_TIMEZONESELECT), TZF_PLF_CB);
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK WorkDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_INFOCHANGED) {
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				if (hContact != NULL) {
					char *szProto = Proto_GetBaseAccountName(hContact);
					if (szProto == nullptr) break;
					SetValue(hwndDlg, IDC_COMPANY, hContact, szProto, "Company", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_DEPARTMENT, hContact, szProto, "CompanyDepartment", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_POSITION, hContact, szProto, "CompanyPosition", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_STREET, hContact, szProto, "CompanyStreet", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_CITY, hContact, szProto, "CompanyCity", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_STATE, hContact, szProto, "CompanyState", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_ZIP, hContact, szProto, "CompanyZIP", SVS_ZEROISUNSPEC);
					SetValue(hwndDlg, IDC_COUNTRY, hContact, szProto, "CompanyCountry", SVS_COUNTRY);
					SetValue(hwndDlg, IDC_WEBPAGE, hContact, szProto, "CompanyHomepage", SVS_ZEROISUNSPEC);
				}
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg), msg, wParam, lParam);
			break;

		case IDC_WEBPAGE:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_WEBPAGE))) {
				char szPage[256];
				GetDlgItemTextA(hwndDlg, IDC_WEBPAGE, szPage, _countof(szPage));
				Utils_OpenUrl(szPage);
			}
		}
		break;
	}
	return FALSE;
}

// Resizes all columns in a listview (report style)
// to make all text visible
void ResizeColumns(HWND hwndLV)
{
	int nCol = 0; LVCOLUMN lvCol;
	lvCol.mask = LVCF_WIDTH;
	while (ListView_GetColumn(hwndLV, nCol++, &lvCol))
		ListView_SetColumnWidth(hwndLV, nCol - 1, LVSCW_AUTOSIZE);
}

static INT_PTR CALLBACK BackgroundDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			RECT rc;
			GetClientRect(GetDlgItem(hwndDlg, IDC_PAST), &rc);
			rc.right -= GetSystemMetrics(SM_CXVSCROLL);

			LVCOLUMN lvc;
			lvc.mask = LVCF_WIDTH;
			lvc.cx = rc.right / 3;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_PAST), 0, &lvc);
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_INTERESTS), 0, &lvc);

			lvc.cx = rc.right - rc.right / 3;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_PAST), 1, &lvc);
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_INTERESTS), 1, &lvc);
		}
		ListView_SetExtendedListViewStyleEx(GetDlgItem(hwndDlg, IDC_PAST), LVS_EX_LABELTIP, LVS_EX_LABELTIP);
		ListView_SetExtendedListViewStyleEx(GetDlgItem(hwndDlg, IDC_INTERESTS), LVS_EX_LABELTIP, LVS_EX_LABELTIP);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_INFOCHANGED) {
				LVITEM lvi;
				char idstr[33];
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				if (hContact != NULL) {
					char *szProto = Proto_GetBaseAccountName(hContact);
					if (szProto == nullptr)
						break;

					SetValue(hwndDlg, IDC_WEBPAGE, hContact, szProto, "Homepage", SVS_ZEROISUNSPEC);

					// past
					ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_PAST));
					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 0;
					lvi.iItem = 0;
					for (int i = 0;; i++) {
						mir_snprintf(idstr, "Past%d", i);
						ptrW tszColText(db_get_wsa(hContact, szProto, idstr));
						if (tszColText == NULL)
							break;
						mir_snprintf(idstr, "Past%dText", i);
						ptrW tszText(db_get_wsa(hContact, szProto, idstr));
						if (tszText == NULL)
							break;

						lvi.pszText = tszColText;
						ListView_InsertItem(GetDlgItem(hwndDlg, IDC_PAST), &lvi);
						ListView_SetItemText(GetDlgItem(hwndDlg, IDC_PAST), lvi.iItem, 1, tszText);
						lvi.iItem++;
					}

					// affiliation
					for (int i = 0;; i++) {
						mir_snprintf(idstr, "Affiliation%d", i);
						ptrW tszColText(db_get_wsa(hContact, szProto, idstr));
						if (tszColText == NULL)
							break;
						mir_snprintf(idstr, "Affiliation%dText", i);
						ptrW tszText(db_get_wsa(hContact, szProto, idstr));
						if (tszText == NULL)
							break;

						lvi.pszText = tszColText;
						ListView_InsertItem(GetDlgItem(hwndDlg, IDC_PAST), &lvi);
						ListView_SetItemText(GetDlgItem(hwndDlg, IDC_PAST), lvi.iItem, 1, tszText);
						lvi.iItem++;
					}

					ResizeColumns(GetDlgItem(hwndDlg, IDC_PAST));

					// interests
					ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_INTERESTS));
					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 0;
					lvi.iItem = 0;
					for (int i = 0;; i++) {
						mir_snprintf(idstr, "Interest%dCat", i);
						ptrW tszColText(db_get_wsa(hContact, szProto, idstr));
						if (tszColText == NULL)
							break;
						mir_snprintf(idstr, "Interest%dText", i);
						ptrW tszText(db_get_wsa(hContact, szProto, idstr));
						if (tszText == NULL)
							break;

						lvi.pszText = tszColText;
						ListView_InsertItem(GetDlgItem(hwndDlg, IDC_INTERESTS), &lvi);
						ListView_SetItemText(GetDlgItem(hwndDlg, IDC_INTERESTS), lvi.iItem, 1, tszText);
						lvi.iItem++;
					}
					ResizeColumns(GetDlgItem(hwndDlg, IDC_INTERESTS));
				}
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg), msg, wParam, lParam);
			break;
		case IDC_WEBPAGE:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_WEBPAGE))) {
				char szPage[256];
				GetDlgItemTextA(hwndDlg, IDC_WEBPAGE, szPage, _countof(szPage));
				Utils_OpenUrl(szPage);
			}
			break;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK NotesDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			HDC hDC = GetDC(hwndDlg);
			LOGFONT lf;
			lf.lfHeight = -MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72);
			ReleaseDC(hwndDlg, hDC);
			lf.lfWidth = 0;
			lf.lfEscapement = 0;
			lf.lfOrientation = 0;
			lf.lfWeight = FW_NORMAL;
			lf.lfItalic = 0;
			lf.lfUnderline = 0;
			lf.lfStrikeOut = 0;
			lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
			lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lf.lfQuality = DEFAULT_QUALITY;
			lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
			mir_wstrcpy(lf.lfFaceName, L"Courier New");
			lf.lfCharSet = DEFAULT_CHARSET;
			HFONT hFont = CreateFontIndirect(&lf);
			SendDlgItemMessage(hwndDlg, IDC_ABOUT, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

			ptrW szNotes(g_plugin.getWStringA(lParam, "MyNotes"));
			if (szNotes != nullptr)
				SetDlgItemText(hwndDlg, IDC_MYNOTES, szNotes);
		}
		SendDlgItemMessage(hwndDlg, IDC_MYNOTES, EM_LIMITTEXT, 2048, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_INFOCHANGED:
				hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				if (hContact != NULL) {
					char *szProto = Proto_GetBaseAccountName(hContact);
					if (szProto != NULL)
						SetValue(hwndDlg, IDC_ABOUT, hContact, szProto, "About", 0);
				}
				break;

			case PSN_APPLY:
				hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				if (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MYNOTES))) {
					wchar_t text[2048];
					GetDlgItemText(hwndDlg, IDC_MYNOTES, text, _countof(text));
					g_plugin.setWString(hContact, "MyNotes", text);
				}
				else g_plugin.delSetting(hContact, "MyNotes");
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
		if (wParam == MAKEWPARAM(IDC_MYNOTES, EN_CHANGE))
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		else if (LOWORD(wParam) == IDCANCEL)
			SendMessage(GetParent(hwndDlg), msg, wParam, lParam);
		break;

	case WM_DESTROY:
		DeleteObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_ABOUT, WM_GETFONT, 0, 0));
		break;
	}
	return FALSE;
}

int DetailsInit(WPARAM wParam, LPARAM lParam)
{
	if (lParam == NULL)
		return 0;

	if (Proto_GetBaseAccountName(lParam) == nullptr)
		return 0;

	OPTIONSDIALOGPAGE odp = {};
	odp.pfnDlgProc = SummaryDlgProc;
	odp.position = -2100000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_SUMMARY);
	odp.szTitle.a = LPGEN("Summary");
	g_plugin.addUserInfo(wParam, &odp);

	odp.pfnDlgProc = ContactDlgProc;
	odp.position = -1800000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_CONTACT);
	odp.szTitle.a = LPGEN("Contact");
	g_plugin.addUserInfo(wParam, &odp);

	odp.pfnDlgProc = LocationDlgProc;
	odp.position = -1500000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_LOCATION);
	odp.szTitle.a = LPGEN("Location");
	g_plugin.addUserInfo(wParam, &odp);

	odp.pfnDlgProc = WorkDlgProc;
	odp.position = -1200000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_WORK);
	odp.szTitle.a = LPGEN("Work");
	g_plugin.addUserInfo(wParam, &odp);

	odp.pfnDlgProc = BackgroundDlgProc;
	odp.position = -900000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_BACKGROUND);
	odp.szTitle.a = LPGEN("Background info");
	g_plugin.addUserInfo(wParam, &odp);

	odp.pfnDlgProc = NotesDlgProc;
	odp.position = 0;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_NOTES);
	odp.szTitle.a = LPGEN("Notes");
	g_plugin.addUserInfo(wParam, &odp);
	return 0;
}
