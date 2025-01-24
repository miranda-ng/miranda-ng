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

/////////////////////////////////////////////////////////////////////////////////////////
// dialog process for more data in the user info window
// lParam = contact handle

class CBriefInfoDlg : public CWeatherDlgBase
{
	static const unsigned tabstops = 48;
	MCONTACT hContact;

	UI_MESSAGE_MAP(CBriefInfoDlg, CWeatherDlgBase);
	UI_MESSAGE(WM_UPDATEDATA, OnUpdate);
	UI_MESSAGE_MAP_END();

	CCtrlButton btnUpdate, btnWebpage, btnToggle;

public:
	CBriefInfoDlg(CWeatherProto *ppro, MCONTACT _1) :
		CWeatherDlgBase(ppro, IDD_BRIEF),
		hContact(_1),
		btnToggle(this, IDC_MTOGGLE),
		btnUpdate(this, IDC_MUPDATE),
		btnWebpage(this, IDC_MWEBPAGE)
	{
		SetMinSize(350, 300);

		btnToggle.OnClick = Callback(this, &CBriefInfoDlg::onClick_Toggle);
		btnUpdate.OnClick = Callback(this, &CBriefInfoDlg::onClick_Update);
		btnWebpage.OnClick = Callback(this, &CBriefInfoDlg::onClick_Webpage);
	}

	bool OnInitDialog() override
	{
		SendDlgItemMessage(m_hwnd, IDC_MTEXT, EM_AUTOURLDETECT, (WPARAM)TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_MTEXT, EM_SETEVENTMASK, 0, ENM_LINK);
		SendDlgItemMessage(m_hwnd, IDC_MTEXT, EM_SETMARGINS, EC_LEFTMARGIN, 5);
		SendDlgItemMessage(m_hwnd, IDC_MTEXT, EM_SETTABSTOPS, 1, (LPARAM)&tabstops);

		// get the list to display
		{
			LV_COLUMN lvc = {};
			HWND hList = GetDlgItem(m_hwnd, IDC_DATALIST);
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
			SendMessage(m_hwnd, WM_UPDATEDATA, 0, 0);
		}
		TranslateDialogDefault(m_hwnd);

		// prevent dups of the window
		WindowList_Add(hDataWindowList, m_hwnd, hContact);

		// restore window position
		Utils_RestoreWindowPositionNoMove(m_hwnd, NULL, MODULENAME, "BriefInfo_");
		return true;
	}

	void OnDestroy() override
	{
		DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, 0));
		DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, 0));

		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "BriefInfo_");
		WindowList_Remove(hDataWindowList, m_hwnd);
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_HEADERBAR:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH;

		case IDC_MTEXT:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

		case IDC_DATALIST:
			{
				HWND hList = GetDlgItem(m_hwnd, IDC_DATALIST);
				RECT rc;
				GetWindowRect(hList, &rc);
				ListView_SetColumnWidth(hList, 1, ListView_GetColumnWidth(hList, 1) + urc->dlgNewSize.cx - (rc.right - rc.left));
			}
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

	INT_PTR OnUpdate(UINT, WPARAM, LPARAM)
	{
		ListView_DeleteAllItems(GetDlgItem(m_hwnd, IDC_DATALIST));
		
		wchar_t str[4096];

		// load weather information from the contact into the WEATHERINFO struct
		WEATHERINFO winfo = m_proto->LoadWeatherInfo(hContact);
		// check if data exist.  If not, display error message box
		if (!m_proto->getByte(hContact, "IsUpdated"))
			SetDlgItemTextW(m_hwnd, IDC_MTEXT, TranslateT("No information available.\r\nPlease update weather condition first."));
		else {
			// set the display text and show the message box
			GetDisplay(&winfo, m_proto->GetTextValue('B'), str);
			SetDlgItemTextW(m_hwnd, IDC_MTEXT, str);
		}

		GetDisplay(&winfo, L"%c, %t", str);
		SetWindowTextW(m_hwnd, winfo.city);
		SetDlgItemTextW(m_hwnd, IDC_HEADERBAR, str);

		DBDataManage(hContact, WDBM_DETAILDISPLAY, (WPARAM)m_hwnd, 0);

		// set icons
		HICON hIcon = m_proto->GetStatusIconBig(hContact);
		DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, LPARAM(hIcon)));
		DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, LPARAM(hIcon)));

		RedrawWindow(GetDlgItem(m_hwnd, IDC_HEADERBAR), nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		return 0;
	}

	void onClick_Update(CCtrlButton *)
	{
		HWND hList = GetDlgItem(m_hwnd, IDC_DATALIST);

		// update current data
		// set the text to "updating"
		SetDlgItemText(m_hwnd, IDC_MTEXT, TranslateT("Retrieving new data, please wait..."));
		ListView_DeleteAllItems(hList);

		LV_ITEM lvi = {};
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.lParam = 1;
		lvi.pszText = L"";
		lvi.iItem = ListView_InsertItem(hList, &lvi);
		lvi.pszText = TranslateT("Retrieving new data, please wait...");
		ListView_SetItemText(hList, lvi.iItem, 1, lvi.pszText);
		m_proto->UpdateSingleStation(hContact, 0);
	}

	void onClick_Webpage(CCtrlButton *)
	{
		m_proto->LoadForecast(hContact, 0);	// read complete forecast
	}

	void onClick_Toggle(CCtrlButton *)
	{
		if (IsWindowVisible(GetDlgItem(m_hwnd, IDC_DATALIST)))
			SetDlgItemText(m_hwnd, IDC_MTOGGLE, TranslateT("More Info"));
		else
			SetDlgItemText(m_hwnd, IDC_MTOGGLE, TranslateT("Brief Info"));
		ShowWindow(GetDlgItem(m_hwnd, IDC_DATALIST), (int)!IsWindowVisible(
			GetDlgItem(m_hwnd, IDC_DATALIST)));
		ShowWindow(GetDlgItem(m_hwnd, IDC_MTEXT), (int)!IsWindowVisible(GetDlgItem(m_hwnd, IDC_MTEXT)));
	}
};

INT_PTR CWeatherProto::BriefInfo(WPARAM hContact, LPARAM)
{
	// make sure that the contact is actually a weather one
	if (!IsMyContact(hContact))
		return 0;

	HWND hMoreDataDlg = WindowList_Find(hDataWindowList, hContact);
	if (hMoreDataDlg != nullptr) {
		SetForegroundWindow(hMoreDataDlg);
		SetFocus(hMoreDataDlg);
	}
	else {
		auto *pDlg = new CBriefInfoDlg(this, hContact);
		pDlg->Create();
		hMoreDataDlg = pDlg->GetHwnd();
	}

	ShowWindow(GetDlgItem(hMoreDataDlg, IDC_DATALIST), 0);
	ShowWindow(GetDlgItem(hMoreDataDlg, IDC_MTEXT), 1);
	SetDlgItemText(hMoreDataDlg, IDC_MTOGGLE, TranslateT("More Info"));
	return 1;
}

int CWeatherProto::BriefInfoEvt(WPARAM wParam, LPARAM)
{
	return BriefInfo(wParam, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// User info dialog

class WeatherUserInfoDlg : public CUserInfoPageDlg
{
	CCtrlButton btnDetail;
	CWeatherProto *ppro;

public:
	WeatherUserInfoDlg() :
		CUserInfoPageDlg(g_plugin, IDD_USERINFO),
		btnDetail(this, IDC_MOREDETAIL)
	{
		btnDetail.OnClick = Callback(this, &WeatherUserInfoDlg::onClick_Detail);
	}

	bool OnInitDialog() override
	{
		SendDlgItemMessage(m_hwnd, IDC_MOREDETAIL, BUTTONSETASFLATBTN, TRUE, 0);
		ppro = (CWeatherProto *)Proto_GetContactInstance(m_hContact);

		// load weather info for the contact
		wchar_t str[MAX_TEXT_SIZE];
		WEATHERINFO w = ppro->LoadWeatherInfo(m_hContact);
		SetDlgItemText(m_hwnd, IDC_INFO1, GetDisplay(&w, TranslateT("Current condition for %n"), str));

		SendDlgItemMessage(m_hwnd, IDC_INFOICON, STM_SETICON, (WPARAM)ppro->GetStatusIconBig(m_hContact), 0);

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
		if (hMoreDataDlg == nullptr) {
			auto *pDlg = new CBriefInfoDlg(ppro, m_hContact);
			pDlg->Create();
			hMoreDataDlg = pDlg->GetHwnd();
		}
		else {
			SetForegroundWindow(hMoreDataDlg);
			SetFocus(hMoreDataDlg);
		}
		ShowWindow(GetDlgItem(hMoreDataDlg, IDC_MTEXT), 0);
		ShowWindow(GetDlgItem(hMoreDataDlg, IDC_DATALIST), 1);
	}
};

int CWeatherProto::UserInfoInit(WPARAM wParam, LPARAM hContact)
{
	// check if it is a weather contact
	if (IsMyContact(hContact)) {
		USERINFOPAGE uip = {};
		uip.szTitle.a = m_szModuleName;
		uip.position = 100000000;
		uip.flags = ODPF_ICON | ODPF_BOLDGROUPS;
		uip.dwInitParam = LPARAM(g_plugin.getIconHandle(IDI_ICON));
		uip.pDialog = new WeatherUserInfoDlg();
		g_plugin.addUserInfo(wParam, &uip);
	}
	return 0;
}
