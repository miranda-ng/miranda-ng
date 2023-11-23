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

/////////////////////////////////////////////////////////////////////////////////////////

CFeedEditor::CFeedEditor(int iItem, CCtrlListView *m_feeds, MCONTACT Contact)
	: CSuper(g_plugin, IDD_ADDFEED),
	m_feedtitle(this, IDC_FEEDTITLE), m_feedurl(this, IDC_FEEDURL),
	m_checktime(this, IDC_CHECKTIME), m_checktimespin(this, IDC_TIMEOUT_VALUE_SPIN, 999),
	m_checkfeed(this, IDC_DISCOVERY), m_useauth(this, IDC_USEAUTH),
	m_login(this, IDC_LOGIN), m_password(this, IDC_PASSWORD),
	m_tagedit(this, IDC_TAGSEDIT), m_reset(this, IDC_RESET),
	m_help(this, IDC_TAGHELP),
	m_iItem(iItem)
{
	m_list = m_feeds;
	m_hContact = Contact;
	m_checkfeed.OnClick = Callback(this, &CFeedEditor::OnCheckFeed);
	m_useauth.OnChange = Callback(this, &CFeedEditor::OnUseAuth);
	m_reset.OnClick = Callback(this, &CFeedEditor::OnReset);
	m_help.OnClick = Callback(this, &CFeedEditor::OnHelp);
}

bool CFeedEditor::OnInitDialog()
{
	if (m_iItem == -1 && m_hContact == NULL)
		SetWindowText(m_hwnd, TranslateT("Add Feed"));
	else
		SetWindowText(m_hwnd, TranslateT("Change Feed"));
	m_checktime.SetMaxLength(3);

	if (m_iItem > -1 && m_hContact == 0) {
		wchar_t SelNick[MAX_PATH], SelUrl[MAX_PACKAGE_NAME];
		m_list->GetItemText(m_iItem, 0, SelNick, _countof(SelNick));
		m_list->GetItemText(m_iItem, 1, SelUrl, _countof(SelNick));

		for (auto &hContact : Contacts(MODULENAME)) {
			ptrW dbNick(g_plugin.getWStringA(hContact, "Nick"));
			if ((dbNick == NULL) || (mir_wstrcmp(dbNick, SelNick) != 0))
				continue;

			ptrW dbURL(g_plugin.getWStringA(hContact, "URL"));
			if ((dbURL == NULL) || (mir_wstrcmp(dbURL, SelUrl) != 0))
				continue;

			m_hContact = hContact;
			m_feedtitle.SetText(SelNick);
			m_feedurl.SetText(SelUrl);
			m_checktime.SetInt(g_plugin.getDword(hContact, "UpdateTime", DEFAULT_UPDATE_TIME));

			ptrW szMsgFormat(g_plugin.getWStringA(hContact, "MsgFormat"));
			if (szMsgFormat)
				m_tagedit.SetText(szMsgFormat);

			if (g_plugin.getByte(hContact, "UseAuth", 0)) {
				m_useauth.SetState(1);
				m_login.Enable();
				m_password.Enable();

				ptrW szLogin(g_plugin.getWStringA(hContact, "Login"));
				if (szLogin)
					m_login.SetText(szLogin);

				pass_ptrA pwd(g_plugin.getStringA(hContact, "Password"));
				m_password.SetTextA(pwd);
			}
			g_arFeeds.insert(this);
			Utils_RestoreWindowPositionNoSize(m_hwnd, hContact, MODULENAME, "ChangeDlg");
			break;
		}
	}
	else if (m_iItem == -1 && m_hContact == NULL) {
		m_feedurl.SetText(L"https://");
		m_tagedit.SetText(TAGSDEFAULT);
		m_checktime.SetInt(DEFAULT_UPDATE_TIME);
		Utils_RestoreWindowPositionNoSize(m_hwnd, NULL, MODULENAME, "AddDlg");
	}
	else if (m_hContact != NULL) {
		ptrW dbNick(g_plugin.getWStringA(m_hContact, "Nick"));
		ptrW dbURL(g_plugin.getWStringA(m_hContact, "URL"));

		m_feedtitle.SetText(dbNick);
		m_feedurl.SetText(dbURL);
		m_checktime.SetInt(g_plugin.getDword(m_hContact, "UpdateTime", DEFAULT_UPDATE_TIME));

		ptrW szMsgFormat(g_plugin.getWStringA(m_hContact, "MsgFormat"));
		if (szMsgFormat)
			m_tagedit.SetText(szMsgFormat);

		if (g_plugin.getByte(m_hContact, "UseAuth")) {
			m_useauth.SetState(1);
			m_login.Enable();
			m_password.Enable();

			ptrW szLogin(g_plugin.getWStringA(m_hContact, "Login"));
			if (szLogin)
				m_login.SetText(szLogin);

			pass_ptrA pwd(g_plugin.getStringA(m_hContact, "Password"));
			m_password.SetTextA(pwd);
		}
		g_arFeeds.insert(this);
		Utils_RestoreWindowPositionNoSize(m_hwnd, m_hContact, MODULENAME, "ChangeDlg");
	}
	return true;
}

void CFeedEditor::OnCheckFeed(CCtrlBase*)
{
	m_checkfeed.Disable();
	m_checkfeed.SetText(TranslateT("Wait..."));
	wchar_t *tszTitle = nullptr;
	ptrW strfeedurl(m_feedurl.GetText());
	if (strfeedurl || mir_wstrcmp(strfeedurl, L"http://") != 0 || mir_wstrcmp(strfeedurl, L"") != 0)
		tszTitle = (wchar_t*)CheckFeed(strfeedurl, this);
	else
		MessageBox(m_hwnd, TranslateT("Enter Feed URL"), TranslateT("Error"), MB_OK);
	m_feedtitle.SetText(tszTitle);
	mir_free(tszTitle);
	m_checkfeed.Enable();
	m_checkfeed.SetText(TranslateT("Check Feed"));
}

void CFeedEditor::OnReset(CCtrlBase*)
{
	if (MessageBox(m_hwnd, TranslateT("Are you sure?"), TranslateT("Tags Mask Reset"), MB_YESNO | MB_ICONWARNING) == IDYES)
		m_tagedit.SetText(TAGSDEFAULT);
}

void CFeedEditor::OnHelp(CCtrlBase*)
{
	CMStringW wszTagHelp;
	wszTagHelp.Format(L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s",
		L"#<title>#", TranslateT("The title of the item."),
		L"#<description>#", TranslateT("The item synopsis."),
		L"#<link>#", TranslateT("The URL of the item."),
		L"#<author>#", TranslateT("Email address of the author of the item."),
		L"#<comments>#", TranslateT("URL of a page for comments relating to the item."),
		L"#<guid>#", TranslateT("A string that uniquely identifies the item."),
		L"#<category>#", TranslateT("Specify one or more categories that the item belongs to."));
	MessageBox(m_hwnd, wszTagHelp, TranslateT("Feed Tag Help"), MB_OK);
}

bool CFeedEditor::OnApply()
{
	ptrW strfeedtitle(m_feedtitle.GetText());
	if (!strfeedtitle || mir_wstrcmp(strfeedtitle, L"") == 0) {
		MessageBox(m_hwnd, TranslateT("Enter Feed name"), TranslateT("Error"), MB_OK);
		return false;
	}

	ptrW strfeedurl(m_feedurl.GetText());
	if (!strfeedurl || mir_wstrcmp(strfeedurl, L"http://") == 0 || mir_wstrcmp(strfeedurl, L"") == 0) {
		MessageBox(m_hwnd, TranslateT("Enter Feed URL"), TranslateT("Error"), MB_OK);
		return false;
	}

	ptrW strtagedit(m_tagedit.GetText());
	if (!strtagedit || mir_wstrcmp(strtagedit, L"") == 0) {
		MessageBox(m_hwnd, TranslateT("Enter message format"), TranslateT("Error"), MB_OK);
		return false;
	}

	MCONTACT hContact;
	if (m_iItem == -1 && m_hContact == NULL) {
		hContact = db_add_contact();
		Proto_AddToContact(hContact, MODULENAME);
		g_plugin.setByte(hContact, "CheckState", 1);
		Contact::Readonly(hContact);
	}
	else hContact = m_hContact;

	g_plugin.setWString(hContact, "Nick", strfeedtitle);
	g_plugin.setWString(hContact, "URL", strfeedurl);
	g_plugin.setDword(hContact, "UpdateTime", m_checktime.GetInt());
	g_plugin.setWString(hContact, "MsgFormat", strtagedit);
	g_plugin.setWord(hContact, "Status", Proto_GetStatus(MODULENAME));
	if (m_useauth.IsChecked()) {
		g_plugin.setByte(hContact, "UseAuth", 1);
		g_plugin.setWString(hContact, "Login", m_login.GetText());
		g_plugin.setString(hContact, "Password", m_password.GetTextA());
	}
	else {
		g_plugin.delSetting(hContact, "UseAuth");
		g_plugin.delSetting(hContact, "Login");
		g_plugin.delSetting(hContact, "Password");
	}

	if (m_iItem == -1 && m_list != nullptr && m_hContact == NULL) {
		int iItem = m_list->AddItem(strfeedtitle, -1);
		m_list->SetItem(iItem, 1, strfeedurl);
		m_list->SetCheckState(iItem, 1);
	}
	else if (m_iItem > -1) {
		m_list->SetItem(m_iItem, 0, strfeedtitle);
		m_list->SetItem(m_iItem, 1, strfeedurl);
	}
	return true;
}

void CFeedEditor::OnDestroy()
{
	g_arFeeds.remove(this);
	Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, m_iItem == -1 ? "AddDlg" : "ChangeDlg");
	if (pAddFeedDialog == this)
		pAddFeedDialog = nullptr;
}

void CFeedEditor::OnUseAuth(CCtrlBase*)
{
	m_login.Enable(m_useauth.GetState());
	m_password.Enable(m_useauth.GetState());
}

void COptionsMain::UpdateList()
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

COptionsMain::COptionsMain() :
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

	m_add.OnClick = Callback(this, &COptionsMain::OnAddButtonClick);
	m_change.OnClick = Callback(this, &COptionsMain::OnChangeButtonClick);
	m_delete.OnClick = Callback(this, &COptionsMain::OnDeleteButtonClick);
	m_import.OnClick = Callback(this, &COptionsMain::OnImportButtonClick);
	m_export.OnClick = Callback(this, &COptionsMain::OnExportButtonClick);

	m_feeds.OnItemChanged = Callback(this, &COptionsMain::OnFeedListItemChanged);
	m_feeds.OnDoubleClick = Callback(this, &COptionsMain::OnFeedListDoubleClick);

}

bool COptionsMain::OnInitDialog()
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

bool COptionsMain::OnApply()
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

void COptionsMain::OnAddButtonClick(CCtrlBase*)
{
	if (pAddFeedDialog == nullptr) {
		pAddFeedDialog = new CFeedEditor(-1, &m_feeds, NULL);
		pAddFeedDialog->SetParent(m_hwnd);
		pAddFeedDialog->Show();
	}
	else {
		SetForegroundWindow(pAddFeedDialog->GetHwnd());
		SetFocus(pAddFeedDialog->GetHwnd());
	}
}

void COptionsMain::OnChangeButtonClick(CCtrlBase*)
{
	int isel = m_feeds.GetSelectionMark();
	CFeedEditor *pDlg = nullptr;
	for (auto &it : g_arFeeds) {
		wchar_t nick[MAX_PATH], url[MAX_PATH];
		m_feeds.GetItemText(isel, 0, nick, _countof(nick));
		m_feeds.GetItemText(isel, 1, url, _countof(url));

		ptrW dbNick(g_plugin.getWStringA(it->getContact(), "Nick"));
		if ((dbNick == NULL) || (mir_wstrcmp(dbNick, nick) != 0))
			continue;

		ptrW dbURL(g_plugin.getWStringA(it->getContact(), "URL"));
		if ((dbURL == NULL) || (mir_wstrcmp(dbURL, url) != 0))
			continue;

		pDlg = it;
	}

	if (pDlg == nullptr) {
		pDlg = new CFeedEditor(isel, &m_feeds, NULL);
		pDlg->SetParent(m_hwnd);
		pDlg->Show();
	}
	else {
		SetForegroundWindow(pDlg->GetHwnd());
		SetFocus(pDlg->GetHwnd());
	}
}

void COptionsMain::OnDeleteButtonClick(CCtrlBase*)
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

			db_delete_contact(hContact, true);
			m_feeds.DeleteItem(isel);
			break;
		}
	}
}

void COptionsMain::OnImportButtonClick(CCtrlBase*)
{
	ImportFeeds(WPARAM(m_hwnd), 0);
}

void COptionsMain::OnExportButtonClick(CCtrlBase*)
{
	ExportFeeds(WPARAM(m_hwnd), 0);
}

void COptionsMain::OnFeedListItemChanged(CCtrlListView::TEventInfo *evt)
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

void COptionsMain::OnFeedListDoubleClick(CCtrlBase*)
{
	int isel = m_feeds.GetHotItem();
	if (isel != -1) {
		CFeedEditor *pDlg = new CFeedEditor(isel, &m_feeds, 0);
		pDlg->SetParent(m_hwnd);
		pDlg->Show();
	}
}

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
