/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/*
This file contain the source that is related to display contact
information, including the one shows in user detail and the brief
information
*/

#include "stdafx.h"

//============  BRIEF INFORMATION  ============
//
static int BriefDlgResizer(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_HEADERBAR:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH;

	case IDC_MTEXT:
	case IDC_DATALIST:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_MUPDATE:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_MTOGGLE:
	case IDC_MWEBPAGE:
	case IDCANCEL:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

// set the title of the dialog and on the which rectangle
// also load brief info into message box
static void LoadBriefInfoText(HWND hwndDlg, MCONTACT hContact)
{
	WEATHERINFO winfo;
	wchar_t str[4096];

	// load weather information from the contact into the WEATHERINFO struct
	winfo = LoadWeatherInfo(hContact);
	// check if data exist.  If not, display error message box
	if (!g_plugin.getByte(hContact, "IsUpdated"))
		wcsncpy(str, TranslateT("No information available.\r\nPlease update weather condition first."), _countof(str) - 1);
	else
		// set the display text and show the message box
		GetDisplay(&winfo, GetTextValue('B'), str);
	SetDlgItemText(hwndDlg, IDC_MTEXT, str);

	GetDisplay(&winfo, L"%c, %t", str);
	SetWindowText(hwndDlg, winfo.city);
	SetDlgItemText(hwndDlg, IDC_HEADERBAR, str);
}

// dialog process for more data in the user info window
// lParam = contact handle
static INT_PTR CALLBACK DlgProcMoreData(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static const unsigned tabstops = 48;
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		// save the contact handle for later use
		hContact = lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hContact);

		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_AUTOURLDETECT, (WPARAM)TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_SETEVENTMASK, 0, ENM_LINK);
		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_SETMARGINS, EC_LEFTMARGIN, 5);
		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_SETTABSTOPS, 1, (LPARAM)&tabstops);

		// get the list to display
		{
			LV_COLUMN lvc = {};
			HWND hList = GetDlgItem(hwndDlg, IDC_DATALIST);
			RECT aRect = {};
			GetClientRect(hList, &aRect);

			// managing styles
			lvc.mask = LVCF_WIDTH | LVCF_TEXT;
			ListView_SetExtendedListViewStyleEx(hList,
				LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP,
				LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

			// inserting columns
			lvc.cx = LIST_COLUMN;
			lvc.pszText = TranslateT("Variable");
			ListView_InsertColumn(hList, 0, &lvc);

			lvc.cx = aRect.right - LIST_COLUMN - GetSystemMetrics(SM_CXVSCROLL) - 3;
			lvc.pszText = TranslateT("Information");
			ListView_InsertColumn(hList, 1, &lvc);

			// inserting data
			SendMessage(hwndDlg, WM_UPDATEDATA, 0, 0);
		}
		TranslateDialogDefault(hwndDlg);

		// prevent dups of the window
		WindowList_Add(hDataWindowList, hwndDlg, hContact);

		// restore window position
		Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, MODULENAME, "BriefInfo_");
		return TRUE;

	case WM_UPDATEDATA:
		ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_DATALIST));
		LoadBriefInfoText(hwndDlg, hContact);
		DBDataManage(hContact, WDBM_DETAILDISPLAY, (WPARAM)hwndDlg, 0);

		// set icons
		{
			HICON hIcon = GetStatusIconBig(hContact);
			DestroyIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, LPARAM(hIcon)));
			DestroyIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, LPARAM(hIcon)));
		}

		RedrawWindow(GetDlgItem(hwndDlg, IDC_HEADERBAR), nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	case WM_SIZE:
		{
			RECT rc;
			HWND hList = GetDlgItem(hwndDlg, IDC_DATALIST);
			GetWindowRect(hList, &rc);
			ListView_SetColumnWidth(hList, 1, ListView_GetColumnWidth(hList, 1) + (int)LOWORD(lParam) - (rc.right - rc.left));

			Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_BRIEF), BriefDlgResizer);
		}
		break;

	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;

			// The minimum width in points
			mmi->ptMinTrackSize.x = 350;
			// The minimum height in points
			mmi->ptMinTrackSize.y = 300;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			// close the info window
			DestroyWindow(hwndDlg);
			break;

		case IDC_MUPDATE:
			{
				HWND hList = GetDlgItem(hwndDlg, IDC_DATALIST);

				// update current data
				// set the text to "updating"
				SetDlgItemText(hwndDlg, IDC_MTEXT, TranslateT("Retrieving new data, please wait..."));
				ListView_DeleteAllItems(hList);

				LV_ITEM lvi = {};
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.lParam = 1;
				lvi.pszText = L"";
				lvi.iItem = ListView_InsertItem(hList, &lvi);
				lvi.pszText = TranslateT("Retrieving new data, please wait...");
				ListView_SetItemText(hList, lvi.iItem, 1, lvi.pszText);
				UpdateSingleStation(hContact, 0);
				break;
			}

		case IDC_MWEBPAGE:
			LoadForecast(hContact, 0);	// read complete forecast
			break;

		case IDC_MTOGGLE:
			if (IsWindowVisible(GetDlgItem(hwndDlg, IDC_DATALIST)))
				SetDlgItemText(hwndDlg, IDC_MTOGGLE, TranslateT("More Info"));
			else
				SetDlgItemText(hwndDlg, IDC_MTOGGLE, TranslateT("Brief Info"));
			ShowWindow(GetDlgItem(hwndDlg, IDC_DATALIST), (int)!IsWindowVisible(
				GetDlgItem(hwndDlg, IDC_DATALIST)));
			ShowWindow(GetDlgItem(hwndDlg, IDC_MTEXT), (int)!IsWindowVisible(GetDlgItem(hwndDlg, IDC_MTEXT)));
			break;
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = (LPNMHDR)lParam;
			if (pNmhdr->idFrom == IDC_MTEXT && pNmhdr->code == EN_LINK) {
				ENLINK *enlink = (ENLINK *)lParam;
				switch (enlink->msg) {
				case WM_LBUTTONUP:
					TEXTRANGE tr;
					tr.chrg = enlink->chrg;
					tr.lpstrText = (wchar_t*)mir_alloc(sizeof(wchar_t)*(tr.chrg.cpMax - tr.chrg.cpMin + 8));
					SendMessage(pNmhdr->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
					Utils_OpenUrlW(tr.lpstrText);
					mir_free(tr.lpstrText);
					break;
				}
			}
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));

		Utils_SaveWindowPosition(hwndDlg, NULL, MODULENAME, "BriefInfo_");
		WindowList_Remove(hDataWindowList, hwndDlg);
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// User info dialog

class WeatherUserInfoDlg : public CUserInfoPageDlg
{
	CCtrlButton btnDetail;

public:
	WeatherUserInfoDlg() :
		CUserInfoPageDlg(g_plugin, IDD_USERINFO),
		btnDetail(this, IDC_MOREDETAIL)
	{
	}

	bool OnInitDialog() override
	{
		SendDlgItemMessage(m_hwnd, IDC_MOREDETAIL, BUTTONSETASFLATBTN, TRUE, 0);

		// load weather info for the contact
		wchar_t str[MAX_TEXT_SIZE];
		WEATHERINFO w = LoadWeatherInfo(m_hContact);
		SetDlgItemText(m_hwnd, IDC_INFO1, GetDisplay(&w, TranslateT("Current condition for %n"), str));

		SendDlgItemMessage(m_hwnd, IDC_INFOICON, STM_SETICON, (WPARAM)GetStatusIconBig(m_hContact), 0);

		// bold and enlarge the current condition
		LOGFONT lf;
		HFONT hNormalFont = (HFONT)SendDlgItemMessage(m_hwnd, IDC_INFO2, WM_GETFONT, 0, 0);
		GetObject(hNormalFont, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		lf.lfWidth = 7;
		lf.lfHeight = 15;
		SendDlgItemMessage(m_hwnd, IDC_INFO2, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);

		// set the text for displaying other current weather conditions data
		GetDisplay(&w, L"%c     %t", str);
		SetDlgItemText(m_hwnd, IDC_INFO2, str);
		SetDlgItemText(m_hwnd, IDC_INFO3, w.feel);
		SetDlgItemText(m_hwnd, IDC_INFO4, w.pressure);
		GetDisplay(&w, L"%i  %w", str);
		SetDlgItemText(m_hwnd, IDC_INFO5, str);
		SetDlgItemText(m_hwnd, IDC_INFO6, w.dewpoint);
		SetDlgItemText(m_hwnd, IDC_INFO7, w.sunrise);
		SetDlgItemText(m_hwnd, IDC_INFO8, w.sunset);
		SetDlgItemText(m_hwnd, IDC_INFO9, w.high);
		SetDlgItemText(m_hwnd, IDC_INFO10, w.low);
		GetDisplay(&w, TranslateT("Last update on:   %u"), str);
		SetDlgItemText(m_hwnd, IDC_INFO11, str);
		SetDlgItemText(m_hwnd, IDC_INFO12, w.humid);
		SetDlgItemText(m_hwnd, IDC_INFO13, w.vis);
		return true;
	}

	void OnDestroy() override
	{
		DestroyIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_INFOICON, STM_SETICON, 0, 0));
		DeleteObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_INFO2, WM_GETFONT, 0, 0));
	}

	void onClick_Detail(CCtrlButton *)
	{
		HWND hMoreDataDlg = WindowList_Find(hDataWindowList, m_hContact);
		if (hMoreDataDlg == nullptr)
			hMoreDataDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_BRIEF), nullptr, DlgProcMoreData, m_hContact);
		else {
			SetForegroundWindow(hMoreDataDlg);
			SetFocus(hMoreDataDlg);
		}
		ShowWindow(GetDlgItem(hMoreDataDlg, IDC_MTEXT), 0);
		ShowWindow(GetDlgItem(hMoreDataDlg, IDC_DATALIST), 1);
	}
};

int UserInfoInit(WPARAM wParam, LPARAM hContact)
{
	USERINFOPAGE uip = {};
	uip.szTitle.a = MODULENAME;
	uip.position = 100000000;

	if (hContact == 0) {
		uip.pDialog = new WeatherMyDetailsDlg();
		g_plugin.addUserInfo(wParam, &uip);
	}
	else if (IsMyContact(hContact)) { // check if it is a weather contact
		uip.pDialog = new WeatherUserInfoDlg();
		uip.flags = ODPF_BOLDGROUPS;
		g_plugin.addUserInfo(wParam, &uip);
	}
	return 0;
}

// show brief information dialog
// wParam = current contact
int BriefInfo(WPARAM wParam, LPARAM)
{
	// make sure that the contact is actually a weather one
	if (!IsMyContact(wParam))
		return 0;

	HWND hMoreDataDlg = WindowList_Find(hDataWindowList, wParam);
	if (hMoreDataDlg != nullptr) {
		SetForegroundWindow(hMoreDataDlg);
		SetFocus(hMoreDataDlg);
	}
	else hMoreDataDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_BRIEF), nullptr, DlgProcMoreData, (LPARAM)wParam);

	ShowWindow(GetDlgItem(hMoreDataDlg, IDC_DATALIST), 0);
	ShowWindow(GetDlgItem(hMoreDataDlg, IDC_MTEXT), 1);
	SetDlgItemText(hMoreDataDlg, IDC_MTOGGLE, TranslateT("More Info"));
	return 1;
}

INT_PTR BriefInfoSvc(WPARAM wParam, LPARAM lParam)
{
	return BriefInfo(wParam, lParam);
}
