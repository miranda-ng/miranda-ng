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

INT_PTR CALLBACK ContactDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define SVS_NORMAL        0
#define SVS_GENDER        1
#define SVS_ZEROISUNSPEC  2
#define SVS_IP            3
#define SVS_COUNTRY       4
#define SVS_MONTH         5
#define SVS_SIGNED        6
#define SVS_TIMEZONE      7

static int Proto_GetContactInfoSetting(MCONTACT hContact, const char *szProto, const char *szModule, const char *szSetting, DBVARIANT *dbv, const int nType)
{
	DBCONTACTGETSETTING cgs = {szModule, szSetting, dbv};
	dbv->type = (BYTE)nType;

	return CallProtoService(szProto, PS_GETINFOSETTING, hContact, (LPARAM)&cgs);
}

static void Proto_FreeInfoVariant(DBVARIANT *dbv)
{
	switch (dbv->type) {
	case DBVT_ASCIIZ:
	case DBVT_UTF8:
	case DBVT_WCHAR:
		mir_free(dbv->pszVal);
		dbv->pszVal = 0;
		break;

	case DBVT_BLOB:
		mir_free(dbv->pbVal);
		dbv->pbVal = 0;
		break;
	}
	dbv->type = 0;
}

static void SetValue(HWND hwndDlg, int idCtrl, MCONTACT hContact, char *szModule, char *szSetting, int special)
{
	char str[80], *pstr = NULL;
	TCHAR *ptstr = NULL;
	char *szProto = GetContactProto(hContact);
	bool proto_service = szProto && (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_INFOSETTINGSVC);

	DBVARIANT dbv = { DBVT_DELETED };

	int unspecified;
	if (szModule == NULL)
		unspecified = 1;
	else if (proto_service)
		unspecified = Proto_GetContactInfoSetting(hContact, szProto, szModule, szSetting, &dbv, 0);
	else
		unspecified = db_get_s(hContact, szModule, szSetting, &dbv, 0);

	if (!unspecified) {
		switch (dbv.type) {
		case DBVT_BYTE:
			if (special == SVS_GENDER) {
				if (dbv.cVal == 'M') ptstr = TranslateT("Male");
				else if (dbv.cVal == 'F') ptstr = TranslateT("Female");
				else unspecified = 1;
			}
			else if (special == SVS_MONTH) {
				if (dbv.bVal > 0 && dbv.bVal <= 12) {
					pstr = str;
					GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SABBREVMONTHNAME1-1 + dbv.bVal, str, SIZEOF(str));
				}
				else unspecified = 1;
			}
			else if (special == SVS_TIMEZONE) {
				if (dbv.cVal == -100) unspecified = 1;
				else {
					pstr = str;
					mir_snprintf(str, SIZEOF(str), dbv.cVal ? "UTC%+d:%02d" : "UTC", -dbv.cVal / 2, (dbv.cVal & 1) * 30);
				}
			}
			else {
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.bVal == 0);
				pstr = _itoa(special == SVS_SIGNED ? dbv.cVal : dbv.bVal, str, 10);
			}
			break;

		case DBVT_WORD:
			if (special == SVS_COUNTRY) {
				WORD wSave = dbv.wVal;
				if (wSave == (WORD)-1) {
					char szSettingName[100];
					mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sName", szSetting);
					if (!db_get_ts(hContact, szModule, szSettingName, &dbv)) {
						ptstr = dbv.ptszVal;
						unspecified = false;
						break;
					}
				}

				pstr = Translate((char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, wSave, 0));
				unspecified = pstr == NULL;
			}
			else {
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.wVal == 0);
				pstr = _itoa(special == SVS_SIGNED ? dbv.sVal : dbv.wVal, str, 10);
			}
			break;

		case DBVT_DWORD:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.dVal == 0);
			if (special == SVS_IP) {
				struct in_addr ia;
				ia.S_un.S_addr = htonl(dbv.dVal);
				pstr = inet_ntoa(ia);
				if (dbv.dVal == 0) unspecified = 1;
			}
			else pstr = _itoa(special == SVS_SIGNED ? dbv.lVal : dbv.dVal, str, 10);
			break;

		case DBVT_ASCIIZ:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
			pstr = dbv.pszVal;
			break;

		case DBVT_UTF8:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
			if (!unspecified) {
				WCHAR *wszStr;
				Utf8Decode(dbv.pszVal, &wszStr);
				SetDlgItemTextW(hwndDlg, idCtrl, TranslateTS(wszStr));
				mir_free(wszStr);
				goto LBL_Exit;
			}

			pstr = dbv.pszVal;
			Utf8Decode(dbv.pszVal, NULL);
			break;

		default:
			pstr = str;
			mir_strcpy(str, "???");
			break;
		}
	}

	if (unspecified)
		SetDlgItemText(hwndDlg, idCtrl, TranslateT("<not specified>"));
	else if (ptstr != NULL)
		SetDlgItemText(hwndDlg, idCtrl, ptstr);
	else
		SetDlgItemTextA(hwndDlg, idCtrl, pstr);

LBL_Exit:
	EnableWindow(GetDlgItem(hwndDlg, idCtrl), !unspecified);
	if (proto_service)
		Proto_FreeInfoVariant(&dbv);
	else
		db_free(&dbv);
}

static INT_PTR CALLBACK SummaryDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_INFOCHANGED) {
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				if (hContact != NULL) {
					char *szProto = GetContactProto(hContact);
					if (szProto == NULL)
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
					SetValue(hwndDlg, IDC_MARITAL, hContact, szProto, "MaritalStatus", 0);
				}
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg), msg, wParam, lParam);
			break;
		case IDC_EMAIL:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EMAIL))) {
				TCHAR szExec[264], szEmail[256];
				GetDlgItemText(hwndDlg, IDC_EMAIL, szEmail, SIZEOF(szEmail));
				mir_sntprintf(szExec, SIZEOF(szExec), _T("mailto:%s"), szEmail);
				ShellExecute(hwndDlg, _T("open"), szExec, NULL, NULL, SW_SHOW);
			}
			break;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK LocationDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		TranslateDialogDefault(hwndDlg);
		SetTimer(hwndDlg, 1, 1000, NULL);

		tmi.prepareList(lParam, NULL, GetDlgItem(hwndDlg, IDC_TIMEZONESELECT), TZF_PLF_CB);
		SendMessage(hwndDlg, WM_TIMER, 0, 0);
		break;

	case WM_TIMER:
		{
			MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (hContact != NULL) {
				TCHAR szTime[80];
				if (tmi.printDateTimeByContact(hContact, _T("s"), szTime, SIZEOF(szTime), TZF_KNOWNONLY)) {
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
					char *szProto = GetContactProto(hContact);
					if (szProto == NULL)
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
		switch(LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg), msg, wParam, lParam);
			break;

		case IDC_TIMEZONESELECT:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				tmi.storeListResults(hContact, NULL, GetDlgItem(hwndDlg, IDC_TIMEZONESELECT), TZF_PLF_CB);
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK WorkDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_INFOCHANGED) {
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				if (hContact != NULL) {
					char *szProto = GetContactProto(hContact);
					if (szProto == NULL) break;
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
		switch(LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg), msg, wParam, lParam);
			break;

		case IDC_WEBPAGE:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_WEBPAGE))) {
				char szPage[256];
				GetDlgItemTextA(hwndDlg, IDC_WEBPAGE, szPage, SIZEOF(szPage));
				CallService(MS_UTILS_OPENURL, 1, (LPARAM)szPage);
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
		ListView_SetColumnWidth(hwndLV, nCol-1, LVSCW_AUTOSIZE);
}

static INT_PTR CALLBACK BackgroundDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{	LVCOLUMN lvc;
			RECT rc;
			GetClientRect(GetDlgItem(hwndDlg, IDC_PAST), &rc);
			rc.right-=GetSystemMetrics(SM_CXVSCROLL);
			lvc.mask = LVCF_WIDTH;
			lvc.cx = rc.right/3;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_PAST), 0, &lvc);
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_INTERESTS), 0, &lvc);
			lvc.cx = rc.right-rc.right/3;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_PAST), 1, &lvc);
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_INTERESTS), 1, &lvc);
		}
		ListView_SetExtendedListViewStyleEx(GetDlgItem(hwndDlg, IDC_PAST), LVS_EX_LABELTIP, LVS_EX_LABELTIP);
		ListView_SetExtendedListViewStyleEx(GetDlgItem(hwndDlg, IDC_INTERESTS), LVS_EX_LABELTIP, LVS_EX_LABELTIP);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_INFOCHANGED)
			{	LVITEM lvi;
				int i;
				char idstr[33];
				DBVARIANT dbv, dbvText;
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;

				if (hContact != NULL) {
					char *szProto = GetContactProto(hContact);
					if (szProto == NULL) break;
					bool proto_service = (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_INFOSETTINGSVC) == PF4_INFOSETTINGSVC;
					SetValue(hwndDlg, IDC_WEBPAGE, hContact, szProto, "Homepage", SVS_ZEROISUNSPEC);

					//past
					ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_PAST));
					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 0;
					lvi.iItem = 0;
					for (i=0;;i++) {
						mir_snprintf(idstr, SIZEOF(idstr), "Past%d", i);
						if ((proto_service && Proto_GetContactInfoSetting(hContact, szProto, szProto, idstr, &dbv, DBVT_TCHAR)) ||
							( !proto_service && db_get_ts(hContact, szProto, idstr, &dbv)))
							break;
						mir_snprintf(idstr, SIZEOF(idstr), "Past%dText", i);
						if (db_get_ts(hContact, szProto, idstr, &dbvText))
						{if (proto_service) Proto_FreeInfoVariant(&dbv); else db_free(&dbv); break;}
						lvi.pszText = dbv.ptszVal;
						ListView_InsertItem(GetDlgItem(hwndDlg, IDC_PAST), &lvi);
						ListView_SetItemText(GetDlgItem(hwndDlg, IDC_PAST), lvi.iItem, 1, dbvText.ptszVal);
						db_free(&dbvText);
						if (proto_service)
							Proto_FreeInfoVariant(&dbv);
						else
							db_free(&dbv);
						lvi.iItem++;
					}

					for (i=0;;i++) {
						mir_snprintf(idstr, SIZEOF(idstr), "Affiliation%d", i);
						if ((proto_service && Proto_GetContactInfoSetting(hContact, szProto, szProto, idstr, &dbv, DBVT_TCHAR)) ||
							( !proto_service && db_get_ts(hContact, szProto, idstr, &dbv)))
							break;
						mir_snprintf(idstr, SIZEOF(idstr), "Affiliation%dText", i);
						if (db_get_ts(hContact, szProto, idstr, &dbvText))
						{if (proto_service) Proto_FreeInfoVariant(&dbv); else db_free(&dbv); break;}
						lvi.pszText = dbv.ptszVal;
						ListView_InsertItem(GetDlgItem(hwndDlg, IDC_PAST), &lvi);
						ListView_SetItemText(GetDlgItem(hwndDlg, IDC_PAST), lvi.iItem, 1, dbvText.ptszVal);
						db_free(&dbvText);
						if (proto_service)
							Proto_FreeInfoVariant(&dbv);
						else
							db_free(&dbv);
						lvi.iItem++;
					}

					ResizeColumns(GetDlgItem(hwndDlg, IDC_PAST));

					//interests
					ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_INTERESTS));
					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 0;
					lvi.iItem = 0;
					for (i=0;;i++) {
						mir_snprintf(idstr, SIZEOF(idstr), "Interest%dCat", i);
						if ((proto_service && Proto_GetContactInfoSetting(hContact, szProto, szProto, idstr, &dbv, DBVT_TCHAR)) ||
							( !proto_service && db_get_ts(hContact, szProto, idstr, &dbv)))
							break;
						mir_snprintf(idstr, SIZEOF(idstr), "Interest%dText", i);
						if (db_get_ts(hContact, szProto, idstr, &dbvText))
						{if (proto_service) Proto_FreeInfoVariant(&dbv); else db_free(&dbv); break;}
						lvi.pszText = dbv.ptszVal;
						ListView_InsertItem(GetDlgItem(hwndDlg, IDC_INTERESTS), &lvi);
						ListView_SetItemText(GetDlgItem(hwndDlg, IDC_INTERESTS), lvi.iItem, 1, dbvText.ptszVal);
						db_free(&dbvText);
						if (proto_service)
							Proto_FreeInfoVariant(&dbv);
						else
							db_free(&dbv);
						lvi.iItem++;
					}
					ResizeColumns(GetDlgItem(hwndDlg, IDC_INTERESTS));
			}	}
			break;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg), msg, wParam, lParam);
			break;
		case IDC_WEBPAGE:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_WEBPAGE))) {
				char szPage[256];
				GetDlgItemTextA(hwndDlg, IDC_WEBPAGE, szPage, SIZEOF(szPage));
				CallService(MS_UTILS_OPENURL, 1, (LPARAM)szPage);
			}
			break;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK NotesDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{	DBVARIANT dbv;
			HFONT hFont;
			LOGFONT lf;
			HDC hDC = GetDC(hwndDlg);
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
			mir_tstrcpy(lf.lfFaceName, _T("Courier New"));
			lf.lfCharSet = DEFAULT_CHARSET;
			hFont = CreateFontIndirect(&lf);
			SendDlgItemMessage(hwndDlg, IDC_ABOUT, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));

			if (!db_get_s(lParam, "UserInfo", "MyNotes", &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_MYNOTES, dbv.pszVal);
				db_free(&dbv);
			}
		}
		SendDlgItemMessage(hwndDlg, IDC_MYNOTES, EM_LIMITTEXT, 2048, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_INFOCHANGED:
				{
					MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
					if (hContact != NULL) {
						char *szProto = GetContactProto(hContact);
						if (szProto != NULL)
							SetValue(hwndDlg, IDC_ABOUT, hContact, szProto, "About", 0);
					}
				}
				break;
			case PSN_APPLY:
				{
					MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
					if (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MYNOTES))) {
						char text[2048];
						GetDlgItemTextA(hwndDlg, IDC_MYNOTES, text, SIZEOF(text));
						db_set_s(hContact, "UserInfo", "MyNotes", text);
					}
					else db_unset(hContact, "UserInfo", "MyNotes");
				}
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
		{
			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_ABOUT, WM_GETFONT, 0, 0);
			DeleteObject(hFont);
		}
		break;
	}
	return FALSE;
}

int DetailsInit(WPARAM wParam, LPARAM lParam)
{
	if (lParam == NULL)
		return 0;

	if (GetContactProto(lParam) == 0)
		return 0;

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;

	odp.pfnDlgProc = SummaryDlgProc;
	odp.position = -2100000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_SUMMARY);
	odp.pszTitle = LPGEN("Summary");
	UserInfo_AddPage(wParam, &odp);

	odp.pfnDlgProc = ContactDlgProc;
	odp.position = -1800000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_CONTACT);
 	odp.pszTitle = LPGEN("Contact");
	UserInfo_AddPage(wParam, &odp);

	odp.pfnDlgProc = LocationDlgProc;
	odp.position = -1500000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_LOCATION);
	odp.pszTitle = LPGEN("Location");
	UserInfo_AddPage(wParam, &odp);

	odp.pfnDlgProc = WorkDlgProc;
	odp.position = -1200000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_WORK);
	odp.pszTitle = LPGEN("Work");
	UserInfo_AddPage(wParam, &odp);

	odp.pfnDlgProc = BackgroundDlgProc;
	odp.position = -900000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_BACKGROUND);
	odp.pszTitle = LPGEN("Background info");
	UserInfo_AddPage(wParam, &odp);

	odp.pfnDlgProc = NotesDlgProc;
	odp.position = 0;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_NOTES);
	odp.pszTitle = LPGEN("Notes");
	UserInfo_AddPage(wParam, &odp);
	return 0;
}
