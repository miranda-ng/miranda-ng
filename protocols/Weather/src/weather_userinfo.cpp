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
// dialog for more data in the user info window

static unsigned tabstops = 48;

static int GetWeatherDataFromDB(const char *szSetting, void *lparam)
{
	auto *pList = (OBJLIST<char>*)lparam;
	pList->insert(newStr(szSetting));
	return 0;
}

class CBriefInfoDlg : public CWeatherDlgBase
{
	MCONTACT hContact;
	wchar_t m_buf[4098];
	int iOldItem = -1;

	UI_MESSAGE_MAP(CBriefInfoDlg, CWeatherDlgBase);
	UI_MESSAGE(WM_UPDATEDATA, OnUpdate);
	UI_MESSAGE_MAP_END();

	CTimer m_timer;
	CCtrlButton btnUpdate, btnWebpage, btnToggle;
	CCtrlListView m_list;

public:
	CBriefInfoDlg(CWeatherProto *ppro, MCONTACT _1) :
		CWeatherDlgBase(ppro, IDD_BRIEF),
		hContact(_1),
		m_list(this, IDC_DATALIST),
		m_timer(this, 1),
		btnToggle(this, IDC_MTOGGLE),
		btnUpdate(this, IDC_MUPDATE),
		btnWebpage(this, IDC_MWEBPAGE)
	{
		SetMinSize(350, 300);

		m_list.OnHotTrack = Callback(this, &CBriefInfoDlg::onList_Track);

		m_timer.OnEvent = Callback(this, &CBriefInfoDlg::onTimer);

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
			RECT aRect = {};
			GetClientRect(m_list.GetHwnd(), &aRect);

			// managing styles
			DWORD dwStyle = LVS_EX_FULLROWSELECT;
			m_list.SetExtendedListViewStyleEx(dwStyle, dwStyle);

			// inserting columns
			LV_COLUMN lvc = {};
			lvc.mask = LVCF_WIDTH | LVCF_TEXT;

			lvc.cx = LIST_COLUMN;
			lvc.pszText = TranslateT("Variable");
			m_list.InsertColumn(0, &lvc);

			lvc.cx = aRect.right - LIST_COLUMN - GetSystemMetrics(SM_CXVSCROLL) - 3;
			lvc.pszText = TranslateT("Information");
			m_list.InsertColumn(1, &lvc);

			// insert data
			OnUpdate();
		}

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
			m_list.SetColumnWidth(1, urc->dlgNewSize.cx - m_list.GetColumnWidth(0) - GetSystemMetrics(SM_CXVSCROLL));
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

	INT_PTR OnUpdate(UINT = 0, WPARAM = 0, LPARAM = 0)
	{
		m_list.DeleteAllItems();
		
		// load weather information from the contact into the WEATHERINFO struct
		WEATHERINFO winfo = m_proto->LoadWeatherInfo(hContact);
		// check if data exist.  If not, display error message box
		if (!m_proto->getByte(hContact, "IsUpdated"))
			SetDlgItemTextW(m_hwnd, IDC_MTEXT, TranslateT("No information available.\r\nPlease update weather condition first."));
		else {
			// set the display text and show the message box
			SetDlgItemTextW(m_hwnd, IDC_MTEXT, GetDisplay(&winfo, m_proto->GetTextValue('B')));
		}

		SetWindowTextW(m_hwnd, winfo.city);
		SetDlgItemTextW(m_hwnd, IDC_HEADERBAR, GetDisplay(&winfo, L"%c, %t"));

		// get all the settings and store them in a temporary list
		LIST<char> arSettings(10);
		db_enum_settings(hContact, GetWeatherDataFromDB, WEATHERCONDITION, &arSettings);

		auto T = arSettings.rev_iter();
		for (auto &it: T) {
			CMStringW wszText(db_get_wsm(hContact, WEATHERCONDITION, it));
			if (wszText.IsEmpty())
				continue;

			// skip the "WeatherInfo" variable
			if (!mir_strcmp(it, "WeatherInfo") || !mir_strcmp(it, "Ignore") || it[0] == '#')
				continue;

			wszText.Replace(L"; ", L";\r\n ");

			_A2T strW(it);
			LV_ITEM lvi = {};
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.lParam = T.indexOf(&it);
			lvi.pszText = TranslateW(strW);
			lvi.iItem = m_list.InsertItem(&lvi);
			lvi.pszText = wszText.GetBuffer();
			m_list.SetItemText(lvi.iItem, 1, wszText);
		}

		// set icons
		HICON hIcon = m_proto->GetStatusIconBig(hContact);
		DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, LPARAM(hIcon)));
		DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, LPARAM(hIcon)));

		RedrawWindow(GetDlgItem(m_hwnd, IDC_HEADERBAR), nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		return 0;
	}

	void onList_Track(CCtrlListView::TEventInfo *ev)
	{
		auto *nlv = ev->nmlv;
		if (nlv->iItem == -1 || nlv->iItem == iOldItem || nlv->iSubItem != 1)
			return;

		iOldItem = nlv->iItem;

		m_list.GetItemText(nlv->iItem, nlv->iSubItem, m_buf, _countof(m_buf));
		m_timer.Start(100);
	}

	void onTimer(CTimer *pTimer)
	{
		pTimer->Stop();

		if (wcslen(m_buf) > 50) {
			CLCINFOTIP ti = {};
			ti.cbSize = sizeof(TOOLINFO);
			ti.hItem = (HANDLE)iOldItem;
			Tipper_ShowTip(m_buf, &ti);
		}
		else Tipper_Hide();
	}

	void onClick_Update(CCtrlButton *)
	{
		// update current data
		// set the text to "updating"
		SetDlgItemText(m_hwnd, IDC_MTEXT, TranslateT("Retrieving new data, please wait..."));
		m_list.DeleteAllItems();

		LV_ITEM lvi = {};
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.lParam = 1;
		lvi.pszText = L"";
		lvi.iItem = m_list.InsertItem(&lvi);
		lvi.pszText = TranslateT("Retrieving new data, please wait...");
		m_list.SetItemText(lvi.iItem, 1, lvi.pszText);
		m_proto->UpdateSingleStation(hContact, 0);
	}

	void onClick_Webpage(CCtrlButton *)
	{
		m_proto->LoadForecast(hContact, 0);	// read complete forecast
	}

	void onClick_Toggle(CCtrlButton *)
	{
		if (IsWindowVisible(m_list.GetHwnd()))
			SetDlgItemText(m_hwnd, IDC_MTOGGLE, TranslateT("More Info"));
		else
			SetDlgItemText(m_hwnd, IDC_MTOGGLE, TranslateT("Brief Info"));
		ShowWindow(m_list.GetHwnd(), (int)!IsWindowVisible(m_list.GetHwnd()));
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
		WEATHERINFO w = ppro->LoadWeatherInfo(m_hContact);
		SetDlgItemText(m_hwnd, IDC_INFO1, GetDisplay(&w, TranslateT("Current condition for %n")));

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
		SetDlgItemText(m_hwnd, IDC_INFO2, GetDisplay(&w, L"%c     %t"));
		SetDlgItemText(m_hwnd, IDC_INFO3, w.feel);
		SetDlgItemText(m_hwnd, IDC_INFO4, w.pressure);
		SetDlgItemText(m_hwnd, IDC_INFO5, GetDisplay(&w, L"%i  %w"));
		SetDlgItemText(m_hwnd, IDC_INFO6, w.dewpoint);
		SetDlgItemText(m_hwnd, IDC_INFO7, w.sunrise);
		SetDlgItemText(m_hwnd, IDC_INFO8, w.sunset);
		SetDlgItemText(m_hwnd, IDC_INFO9, w.high);
		SetDlgItemText(m_hwnd, IDC_INFO10, w.low);
		SetDlgItemText(m_hwnd, IDC_INFO11, GetDisplay(&w, TranslateT("Last update on:   %u")));
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
		uip.szTitle.w = m_tszUserName;
		uip.position = 100000000;
		uip.flags = ODPF_ICON | ODPF_BOLDGROUPS | ODPF_UNICODE;
		uip.dwInitParam = LPARAM(g_plugin.getIconHandle(IDI_ICON));
		uip.pDialog = new WeatherUserInfoDlg();
		g_plugin.addUserInfo(wParam, &uip);
	}
	return 0;
}
