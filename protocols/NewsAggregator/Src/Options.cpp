/*
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

class COptionsMain : public CDlgBase
{
	CCtrlListView m_feeds;
	CCtrlButton m_add;
	CCtrlButton m_change;
	CCtrlButton m_delete;
	CCtrlButton m_import;
	CCtrlButton m_export;
	CCtrlCheck m_checkonstartup;

public:
	COptionsMain() :
		CDlgBase(g_plugin, IDD_OPTIONS),
		m_feeds(this, IDC_FEEDLIST),
		m_add(this, IDC_ADD),
		m_change(this, IDC_CHANGE),
		m_delete(this, IDC_REMOVE),
		m_import(this, IDC_IMPORT),
		m_export(this, IDC_EXPORT),
		m_checkonstartup(this, IDC_STARTUPRETRIEVE)
	{
		CreateLink(m_checkonstartup, "StartupRetrieve", DBVT_BYTE, 1);

		m_add.OnClick = Callback(this, &COptionsMain::onClick_Add);
		m_change.OnClick = Callback(this, &COptionsMain::onClick_Change);
		m_delete.OnClick = Callback(this, &COptionsMain::onClick_Delete);
		m_import.OnClick = Callback(this, &COptionsMain::onClick_Import);
		m_export.OnClick = Callback(this, &COptionsMain::onClick_Export);

		m_feeds.OnItemChanged = Callback(this, &COptionsMain::ontItemChanged_Feeds);
		m_feeds.OnDoubleClick = Callback(this, &COptionsMain::onDoubleClick_Feeds);
	}

	bool OnInitDialog() override
	{
		CDlgBase::OnInitDialog();
		m_change.Disable();
		m_delete.Disable();
		m_feeds.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		m_feeds.AddColumn(0, TranslateT("Feed"), 160);
		m_feeds.AddColumn(1, TranslateT("URL"), 276);
		UpdateList();
		return true;
	}

	bool OnApply() override
	{
		for (auto &hContact : Contacts(MODULENAME)) {
			ptrW dbNick(g_plugin.getWStringA(hContact, "Nick"));
			for (int i = 0; i < m_feeds.GetItemCount(); i++) {
				wchar_t nick[MAX_PATH];
				m_feeds.GetItemText(i, 0, nick, _countof(nick));
				if (mir_wstrcmp(dbNick, nick) == 0) {
					g_plugin.setByte(hContact, "CheckState", m_feeds.GetCheckState(i));
					if (!m_feeds.GetCheckState(i))
						Contact::Hide(hContact);
					else
						Contact::Hide(hContact, false);
				}
			}
		}
		return true;
	}

	void onClick_Add(CCtrlBase *)
	{
		AddFeed(0, LPARAM(&m_feeds));
	}

	void onClick_Change(CCtrlBase *)
	{
		int isel = m_feeds.GetSelectionMark();
		wchar_t nick[MAX_PATH], url[MAX_PATH];
		m_feeds.GetItemText(isel, 0, nick, _countof(nick));
		m_feeds.GetItemText(isel, 1, url, _countof(url));

		auto *pDlg = FindFeedEditor(nick, url);
		if (pDlg == nullptr) {
			pDlg = new CFeedEditor(isel, &m_feeds, NULL);
			pDlg->Show();
		}
		else {
			SetForegroundWindow(pDlg->GetHwnd());
			SetFocus(pDlg->GetHwnd());
		}
	}

	void onClick_Delete(CCtrlBase *)
	{
		if (MessageBox(m_hwnd, TranslateT("Are you sure?"), TranslateT("Contact deleting"), MB_YESNO | MB_ICONWARNING) == IDYES) {
			wchar_t nick[MAX_PATH], url[MAX_PATH];
			int isel = m_feeds.GetSelectionMark();
			m_feeds.GetItemText(isel, 0, nick, _countof(nick));
			m_feeds.GetItemText(isel, 1, url, _countof(url));

			for (auto &hContact : Contacts(MODULENAME)) {
				ptrW dbNick(g_plugin.getWStringA(hContact, "Nick"));
				if (dbNick == NULL)
					break;
				if (mir_wstrcmp(dbNick, nick))
					continue;

				ptrW dbURL(g_plugin.getWStringA(hContact, "URL"));
				if (dbURL == NULL)
					break;
				if (mir_wstrcmp(dbURL, url))
					continue;

				db_delete_contact(hContact, CDF_FROM_SERVER);
				m_feeds.DeleteItem(isel);
				break;
			}
		}
	}

	void onClick_Import(CCtrlBase *)
	{
		ImportFeeds(WPARAM(m_hwnd), 0);
	}

	void onClick_Export(CCtrlBase *)
	{
		ExportFeeds(WPARAM(m_hwnd), 0);
	}

	void ontItemChanged_Feeds(CCtrlListView::TEventInfo *evt)
	{
		int isel = m_feeds.GetSelectionMark();
		if (isel == -1) {
			m_change.Disable();
			m_delete.Disable();
		}
		else {
			m_change.Enable();
			m_delete.Enable();
		}
		if (((evt->nmlv->uNewState ^ evt->nmlv->uOldState) & LVIS_STATEIMAGEMASK) && !UpdateListFlag)
			NotifyChange();
	}

	void onDoubleClick_Feeds(CCtrlBase *)
	{
		int isel = m_feeds.GetHotItem();
		if (isel != -1) {
			CFeedEditor *pDlg = new CFeedEditor(isel, &m_feeds, 0);
			pDlg->Show();
		}
	}

	void UpdateList()
	{
		for (auto &hContact : Contacts(MODULENAME)) {
			UpdateListFlag = TRUE;
			ptrW ptszNick(g_plugin.getWStringA(hContact, "Nick"));
			if (ptszNick) {
				int iItem = m_feeds.AddItem(ptszNick, -1);

				ptrW ptszURL(g_plugin.getWStringA(hContact, "URL"));
				if (ptszURL) {
					m_feeds.SetItem(iItem, 1, ptszURL);
					m_feeds.SetCheckState(iItem, g_plugin.getByte(hContact, "CheckState", 1));
				}
			}
		}
		UpdateListFlag = FALSE;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = LPGENW("News Aggregator");
	odp.pDialog = new COptionsMain();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
