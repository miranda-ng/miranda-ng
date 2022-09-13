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

static HFONT hEmailFont = nullptr;
static HCURSOR hHandCursor = nullptr;

static INT_PTR CALLBACK EditUserEmailDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		if (*(char *)lParam)
			SetWindowText(hwndDlg, TranslateT("Edit e-mail address"));
		TranslateDialogDefault(hwndDlg);
		SetDlgItemTextA(hwndDlg, IDC_EMAIL, (char *)lParam);
		EnableWindow(GetDlgItem(hwndDlg, IDOK), *(char *)lParam);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemTextA(hwndDlg, IDC_EMAIL, (char *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA), 256);
			__fallthrough;

		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			__fallthrough;

		case IDC_EMAIL:
			if (HIWORD(wParam) == EN_CHANGE)
				EnableWindow(GetDlgItem(hwndDlg, IDOK), GetWindowTextLength(GetDlgItem(hwndDlg, IDC_EMAIL)));
			break;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK EditUserPhoneDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int noRecursion = 0;

	switch (msg) {
	case WM_INITDIALOG:
		{
			char *szText = (char *)lParam;
			int i, item, countryCount;
			struct CountryListEntry *countries;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			if (szText[0]) SetWindowText(hwndDlg, TranslateT("Edit phone number"));
			TranslateDialogDefault(hwndDlg);
			if (mir_strlen(szText) > 4 && !mir_strcmp(szText + mir_strlen(szText) - 4, " SMS")) {
				CheckDlgButton(hwndDlg, IDC_SMS, BST_CHECKED);
				szText[mir_strlen(szText) - 4] = '\0';
			}
			EnableWindow(GetDlgItem(hwndDlg, IDOK), szText[0]);
			SendDlgItemMessage(hwndDlg, IDC_AREA, EM_LIMITTEXT, 31, 0);
			SendDlgItemMessage(hwndDlg, IDC_NUMBER, EM_LIMITTEXT, 63, 0);
			CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&countryCount, (LPARAM)&countries);
			for (i = 0; i < countryCount; i++) {
				if (countries[i].id == 0 || countries[i].id == 0xFFFF) continue;
				item = SendDlgItemMessageA(hwndDlg, IDC_COUNTRY, CB_ADDSTRING, 0, (LPARAM)Translate(countries[i].szName));
				SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_SETITEMDATA, item, countries[i].id);
			}
			SetDlgItemTextA(hwndDlg, IDC_PHONE, szText);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				char *szText = (char *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				int isValid = 1;
				GetDlgItemTextA(hwndDlg, IDC_PHONE, szText, 252);
				if (mir_strlen(szText) < 7 || szText[0] != '+') isValid = 0;
				if (isValid) isValid = (mir_strlen(szText + 1) == (int)strspn(szText + 1, "0123456789 ()-"));
				if (!isValid) {
					MessageBox(hwndDlg, TranslateT("The phone number should start with a + and consist of numbers, spaces, brackets and hyphens only."), TranslateT("Invalid phone number"), MB_OK);
					break;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_SMS)) mir_strcat(szText, " SMS");
			}
			__fallthrough;

		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			__fallthrough;

		case IDC_COUNTRY:
			if (HIWORD(wParam) != CBN_SELCHANGE)
				break;
		case IDC_AREA:
		case IDC_NUMBER:
			if (LOWORD(wParam) != IDC_COUNTRY && HIWORD(wParam) != EN_CHANGE) break;
			if (noRecursion) break;
			EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
			{
				char szPhone[96], szArea[32], szNumber[64];
				GetDlgItemTextA(hwndDlg, IDC_AREA, szArea, _countof(szArea));
				GetDlgItemTextA(hwndDlg, IDC_NUMBER, szNumber, _countof(szNumber));
				mir_snprintf(szPhone, "+%u (%s) %s", SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_GETCURSEL, 0, 0), 0), szArea, szNumber);
				noRecursion = 1;
				SetDlgItemTextA(hwndDlg, IDC_PHONE, szPhone);
				noRecursion = 0;
			}
			break;
		case IDC_PHONE:
			if (HIWORD(wParam) != EN_UPDATE) break;
			if (noRecursion) break;
			noRecursion = 1;
			{
				char szText[256], *pText = nullptr, *pArea, *pNumber;
				int i, isValid = 1;
				GetDlgItemTextA(hwndDlg, IDC_PHONE, szText, _countof(szText));
				if (szText[0] != '+')
					isValid = 0;

				if (isValid) {
					int country = strtol(szText + 1, &pText, 10);
					if (pText - szText > 4)
						isValid = 0;
					else {
						for (i = SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_GETCOUNT, 0, 0) - 1; i >= 0; i--)
							if (country == SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_GETITEMDATA, i, 0)) {
								SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_SETCURSEL, i, 0);
								break;
							}
						if (i < 0)
							isValid = 0;
					}
				}
				if (isValid) {
					pArea = pText + strcspn(pText, "0123456789");
					pText = pArea + strspn(pArea, "0123456789");
					if (*pText) {
						*pText = '\0';
						pNumber = pText + 1 + strcspn(pText + 1, "0123456789");
						SetDlgItemTextA(hwndDlg, IDC_NUMBER, pNumber);
					}
					SetDlgItemTextA(hwndDlg, IDC_AREA, pArea);
				}
				if (!isValid) {
					SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_SETCURSEL, -1, 0);
					SetDlgItemTextA(hwndDlg, IDC_AREA, "");
					SetDlgItemTextA(hwndDlg, IDC_NUMBER, "");
				}
			}
			noRecursion = 0;
			EnableWindow(GetDlgItem(hwndDlg, IDOK), GetWindowTextLength(GetDlgItem(hwndDlg, IDC_PHONE)));
			break;
		}
		break;
	}
	return FALSE;
}

static int IsOverEmail(HWND hwndDlg, wchar_t *szEmail, int cchEmail)
{
	HWND hwndEmails = GetDlgItem(hwndDlg, IDC_EMAILS);

	LVHITTESTINFO hti;
	GetCursorPos(&hti.pt);
	ScreenToClient(hwndEmails, &hti.pt);

	RECT rc;
	GetClientRect(hwndEmails, &rc);
	if (!PtInRect(&rc, hti.pt))
		return 0;
	if (ListView_SubItemHitTest(hwndEmails, &hti) == -1)
		return 0;
	if (hti.iSubItem != 1 || !(hti.flags & LVHT_ONITEMLABEL))
		return 0;

	ListView_GetSubItemRect(hwndEmails, hti.iItem, 1, LVIR_LABEL, &rc);

	wchar_t szText[256]; szText[0] = 0;
	ListView_GetItemText(hwndEmails, hti.iItem, 1, szText, _countof(szText));

	HDC hdc = GetDC(hwndEmails);
	SelectObject(hdc, hEmailFont);

	SIZE textSize;
	GetTextExtentPoint32(hdc, szText, (int)mir_wstrlen(szText), &textSize);
	ReleaseDC(hwndEmails, hdc);
	if (hti.pt.x < rc.left + textSize.cx) {
		if (szEmail && cchEmail)
			mir_wstrncpy(szEmail, szText, cchEmail);
		return 1;
	}
	return 0;
}

static DataItem itemsContact[] = {
	{ "e-mail",       0, 0 },
	{ "e-mail0",      0, 0 },
	{ "Mye-mail0",    0, 0 },
	{ "Phone",        0, 0 },
	{ "Fax",          0, 0 },
	{ "Cellular",     0, 0 },
	{ "CompanyPhone", 0, 0 },
	{ "CompanyFax",   0, 0 },
	{ "MyPhone0",     0, 0 },
};

class CContactPage : public CUserInfoPageDlg
{
	typedef CUserInfoPageDlg CSuper;

	CCtrlListView m_emails, m_phones;

public:
	CContactPage() :
		CSuper(g_plugin, IDD_INFO_CONTACT),
		m_emails(this, IDC_EMAILS),
		m_phones(this, IDC_PHONES)
	{
		m_emails.OnCustomDraw = m_phones.OnCustomDraw = Callback(this, &CContactPage::onCustomDraw);
	}

	bool OnInitDialog() override
	{
		if (hEmailFont)
			DeleteObject(hEmailFont);
		hEmailFont = (HFONT)m_emails.SendMsg(WM_GETFONT, 0, 0);

		LOGFONT lf;
		GetObject(hEmailFont, sizeof(lf), &lf);
		lf.lfUnderline = 1;
		hEmailFont = CreateFontIndirect(&lf);

		if (hHandCursor == nullptr)
			hHandCursor = LoadCursor(nullptr, IDC_HAND);

		m_emails.SetExtendedListViewStyleEx(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
		m_phones.SetExtendedListViewStyleEx(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

		RECT rc;
		GetClientRect(m_emails.GetHwnd(), &rc);
		rc.right -= GetSystemMetrics(SM_CXVSCROLL);

		LVCOLUMN lvc;
		lvc.mask = LVCF_WIDTH;
		lvc.cx = 50;
		m_emails.InsertColumn(0, &lvc);
		m_phones.InsertColumn(0, &lvc);
		
		lvc.cx = rc.right - m_emails.GetColumnWidth(0);
		m_emails.InsertColumn(1, &lvc);

		lvc.cx = rc.right - m_phones.GetColumnWidth(0);
		m_phones.InsertColumn(1, &lvc);
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_EMAILS:
		case IDC_PHONES:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
		}

		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	bool IsEmpty() const override
	{
		return IsEmptyValue(m_hContact, itemsContact);
	}

	bool OnRefresh() override
	{
		if (m_hContact == 0)
			return false;

		char *szProto = Proto_GetBaseAccountName(m_hContact);
		if (szProto == nullptr)
			return false;

		// e-mails
		m_emails.DeleteAllItems();

		char idstr[33];
		wchar_t idstr2[33];
		DBVARIANT dbv;

		LVITEM lvi;
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.lParam = -1;
		lvi.iSubItem = 0;
		lvi.iItem = 0;
		for (int i = -1;; i++) {
			if (i == -1) {
				if (db_get_ws(m_hContact, szProto, "e-mail", &dbv))
					continue;
				lvi.pszText = TranslateT("Primary");
			}
			else {
				mir_snprintf(idstr, "e-mail%d", i);
				if (db_get_ws(m_hContact, szProto, idstr, &dbv))
					break;

				lvi.pszText = idstr2;
				mir_snwprintf(idstr2, L"%d", i + 2);
			}
			m_emails.InsertItem(&lvi);
			m_emails.SetItemText(lvi.iItem, 1, dbv.pwszVal);
			db_free(&dbv);
			lvi.iItem++;
		}
		lvi.iSubItem = 0;
		for (int i = 0;; i++) {
			lvi.lParam = i;
			mir_snprintf(idstr, "Mye-mail%d", i);
			if (g_plugin.getWString(m_hContact, idstr, &dbv))
				break;
			lvi.pszText = idstr2;
			mir_snwprintf(idstr2, TranslateT("Custom %d"), i + 1);
			m_emails.InsertItem(&lvi);
			m_emails.SetItemText(lvi.iItem, 1, dbv.pwszVal);
			db_free(&dbv);
			lvi.iItem++;
		}
		lvi.mask = LVIF_PARAM;
		lvi.lParam = -2;
		m_emails.InsertItem(&lvi);

		// phones
		m_phones.DeleteAllItems();

		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.lParam = -1;
		lvi.iSubItem = 0;
		lvi.iItem = 0;
		if (!db_get_ws(m_hContact, szProto, "Phone", &dbv)) {
			lvi.pszText = TranslateT("Primary");
			m_phones.InsertItem(&lvi);
			m_phones.SetItemText(lvi.iItem, 1, dbv.pwszVal);
			db_free(&dbv);
			lvi.iItem++;
		}
		if (!db_get_ws(m_hContact, szProto, "Fax", &dbv)) {
			lvi.pszText = TranslateT("Fax");
			m_phones.InsertItem(&lvi);
			m_phones.SetItemText(lvi.iItem, 1, dbv.pwszVal);
			db_free(&dbv);
			lvi.iItem++;
		}
		if (!db_get_ws(m_hContact, szProto, "Cellular", &dbv)) {
			lvi.pszText = TranslateT("Mobile");
			m_phones.InsertItem(&lvi);
			if (mir_strlen(dbv.pszVal) > 4 && !mir_strcmp(dbv.pszVal + mir_strlen(dbv.pszVal) - 4, " SMS")) {
				m_phones.SetItemText(lvi.iItem, 2, L"y");
				dbv.pwszVal[mir_wstrlen(dbv.pwszVal) - 4] = '\0';
			}
			m_phones.SetItemText(lvi.iItem, 1, dbv.pwszVal);
			db_free(&dbv);
			lvi.iItem++;
		}
		if (!db_get_ws(m_hContact, szProto, "CompanyPhone", &dbv)) {
			lvi.pszText = TranslateT("Work phone");
			m_phones.InsertItem(&lvi);
			m_phones.SetItemText(lvi.iItem, 1, dbv.pwszVal);
			db_free(&dbv);
			lvi.iItem++;
		}
		if (!db_get_ws(m_hContact, szProto, "CompanyFax", &dbv)) {
			lvi.pszText = TranslateT("Work fax");
			m_phones.InsertItem(&lvi);
			m_phones.SetItemText(lvi.iItem, 1, dbv.pwszVal);
			db_free(&dbv);
			lvi.iItem++;
		}
		lvi.iSubItem = 0;
		for (int i = 0;; i++) {
			lvi.lParam = i;
			mir_snprintf(idstr, "MyPhone%d", i);
			if (g_plugin.getWString(m_hContact, idstr, &dbv))
				break;
			lvi.pszText = idstr2;
			mir_snwprintf(idstr2, TranslateT("Custom %d"), i + 1);
			m_phones.InsertItem(&lvi);
			if (mir_wstrlen(dbv.pwszVal) > 4 && !mir_wstrcmp(dbv.pwszVal + mir_wstrlen(dbv.pwszVal) - 4, L" SMS")) {
				m_phones.SetItemText(lvi.iItem, 2, L"y");
				dbv.pwszVal[mir_wstrlen(dbv.pwszVal) - 4] = '\0';
			}
			m_phones.SetItemText(lvi.iItem, 1, dbv.pwszVal);
			db_free(&dbv);
			lvi.iItem++;
		}
		lvi.mask = LVIF_PARAM;
		lvi.lParam = -2;
		m_phones.InsertItem(&lvi);
		return false;
	}

	void onCustomDraw(CCtrlListView::TEventInfo *ev)
	{
		NMLVCUSTOMDRAW *nm = ev->nmcd;
		switch (nm->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
		case CDDS_ITEMPREPAINT:
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW);
			break;

		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
			RECT rc;
			ListView_GetSubItemRect(nm->nmcd.hdr.hwndFrom, nm->nmcd.dwItemSpec, nm->iSubItem, LVIR_LABEL, &rc);
			if (nm->iSubItem == 1 && nm->nmcd.hdr.idFrom == IDC_EMAILS) {
				HFONT hoFont;
				wchar_t szText[256] = { 0 };
				ListView_GetItemText(nm->nmcd.hdr.hwndFrom, nm->nmcd.dwItemSpec, nm->iSubItem, szText, _countof(szText));
				hoFont = (HFONT)SelectObject(nm->nmcd.hdc, hEmailFont);
				SetTextColor(nm->nmcd.hdc, RGB(0, 0, 255));
				DrawText(nm->nmcd.hdc, szText, -1, &rc, DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);
				SelectObject(nm->nmcd.hdc, hoFont);
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
				break;
			}

			HICON hIcon = NULL;
			if (nm->nmcd.lItemlParam == -2 && nm->iSubItem - 3 == (nm->nmcd.hdr.idFrom == IDC_PHONES))
				hIcon = Skin_LoadIcon(SKINICON_OTHER_ADDCONTACT);
			else if (nm->iSubItem > 1 && nm->nmcd.lItemlParam != -1 && nm->nmcd.lItemlParam != -2) {
				static int iconResources[3] = { SKINICON_OTHER_RENAME, SKINICON_OTHER_DELETE };
				if (nm->iSubItem == 2 && nm->nmcd.hdr.idFrom == IDC_PHONES) {
					wchar_t szText[2];
					ListView_GetItemText(nm->nmcd.hdr.hwndFrom, nm->nmcd.dwItemSpec, nm->iSubItem, szText, _countof(szText));
					if (szText[0]) hIcon = Skin_LoadIcon(SKINICON_OTHER_SMS);
				}
				else hIcon = Skin_LoadIcon(iconResources[nm->iSubItem - 3 + (nm->nmcd.hdr.idFrom == IDC_EMAILS)]);
			}
			else break;
			DrawIconEx(nm->nmcd.hdc, (rc.left + rc.right - GetSystemMetrics(SM_CXSMICON)) / 2, (rc.top + rc.bottom - GetSystemMetrics(SM_CYSMICON)) / 2, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
			IcoLib_ReleaseIcon(hIcon, 0);
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
		}
	}

	void onClick(CCtrlListView::TEventInfo *ev)
	{
		NMLISTVIEW *nm = ev->nmlv;
		char *szIdTemplate = (nm->hdr.idFrom == IDC_PHONES) ? "MyPhone%d" : "Mye-mail%d";
		int i;

		wchar_t szEmail[256];
		if (IsOverEmail(m_hwnd, szEmail, _countof(szEmail))) {
			wchar_t szExec[264];
			mir_snwprintf(szExec, L"mailto:%s", szEmail);
			ShellExecute(m_hwnd, L"open", szExec, NULL, NULL, SW_SHOW);
			return;
		}
		if (nm->iSubItem < 2)
			return;

		LVHITTESTINFO hti;
		hti.pt.x = (short)LOWORD(GetMessagePos());
		hti.pt.y = (short)HIWORD(GetMessagePos());
		ScreenToClient(nm->hdr.hwndFrom, &hti.pt);
		if (ListView_SubItemHitTest(nm->hdr.hwndFrom, &hti) == -1)
			return;

		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iItem = hti.iItem;
		lvi.iSubItem = 0;
		ListView_GetItem(nm->hdr.hwndFrom, &lvi);
		if (lvi.lParam == -1)
			return;

		if (lvi.lParam == -2) {
			if (hti.iSubItem - 3 == (nm->hdr.idFrom == IDC_PHONES)) {
				// add
				char szNewData[256] = "", idstr[33];
				DBVARIANT dbv;
				if (IDOK != DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(nm->hdr.idFrom == IDC_PHONES ? IDD_ADDPHONE : IDD_ADDEMAIL), m_hwnd, nm->hdr.idFrom == IDC_PHONES ? EditUserPhoneDlgProc : EditUserEmailDlgProc, (LPARAM)szNewData))
					return;

				for (i = 0;; i++) {
					mir_snprintf(idstr, szIdTemplate, i);
					if (g_plugin.getString(m_hContact, idstr, &dbv)) break;
					db_free(&dbv);
				}
				g_plugin.setString(m_hContact, idstr, szNewData);
				OnRefresh();
			}
		}
		else if (hti.iSubItem - 3 == (nm->hdr.idFrom == IDC_PHONES)) {
			// delete
			char idstr[33];
			DBVARIANT dbv;
			for (i = lvi.lParam;; i++) {
				mir_snprintf(idstr, szIdTemplate, i + 1);
				if (g_plugin.getString(m_hContact, idstr, &dbv)) break;
				mir_snprintf(idstr, szIdTemplate, i);
				g_plugin.setString(m_hContact, idstr, dbv.pszVal);
				db_free(&dbv);
			}
			mir_snprintf(idstr, szIdTemplate, i);
			g_plugin.delSetting(m_hContact, idstr);
			OnRefresh();
		}
		else if (hti.iSubItem - 2 == (nm->hdr.idFrom == IDC_PHONES)) {
			// edit
			char szText[256], idstr[33];
			DBVARIANT dbv;
			mir_snprintf(idstr, szIdTemplate, lvi.lParam);
			if (g_plugin.getString(m_hContact, idstr, &dbv))
				return;
			mir_strncpy(szText, dbv.pszVal, _countof(szText));
			db_free(&dbv);
			if (IDOK != DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(nm->hdr.idFrom == IDC_PHONES ? IDD_ADDPHONE : IDD_ADDEMAIL), m_hwnd, nm->hdr.idFrom == IDC_PHONES ? EditUserPhoneDlgProc : EditUserEmailDlgProc, (LPARAM)szText))
				return;
			g_plugin.setString(m_hContact, idstr, szText);
			OnRefresh();
		}
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_SETCURSOR && LOWORD(lParam) == HTCLIENT) {
			if (IsOverEmail(m_hwnd, nullptr, 0)) {
				SetCursor(hHandCursor);
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

void InitContactInfo(WPARAM wParam, USERINFOPAGE &uip)
{
	uip.pDialog = new CContactPage();
	uip.szTitle.a = LPGEN("Contact");
	uip.dwInitParam = (INT_PTR)g_plugin.getIconHandle(IDI_CONTACT);
	g_plugin.addUserInfo(wParam, &uip);
}
