/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static CDlgBase *pAddFeedDialog = nullptr;

static LIST<class CFeedEditor> g_arFeeds(1, PtrKeySortT);

/////////////////////////////////////////////////////////////////////////////////////////

CFeedEditor::CFeedEditor(int iItem, CCtrlListView *feeds, MCONTACT Contact) :
	CSuper(g_plugin, IDD_ADDFEED),
	m_feedtitle(this, IDC_FEEDTITLE), m_feedurl(this, IDC_FEEDURL),
	m_checktime(this, IDC_CHECKTIME), m_checktimespin(this, IDC_TIMEOUT_VALUE_SPIN, 999),
	m_checkfeed(this, IDC_DISCOVERY), m_useauth(this, IDC_USEAUTH),
	m_login(this, IDC_LOGIN), m_password(this, IDC_PASSWORD),
	m_tagedit(this, IDC_TAGSEDIT), m_reset(this, IDC_RESET),
	m_help(this, IDC_TAGHELP),
	m_iItem(iItem)
{
	m_list = feeds;
	m_hContact = Contact;
	m_checkfeed.OnClick = Callback(this, &CFeedEditor::OnCheckFeed);
	m_useauth.OnChange = Callback(this, &CFeedEditor::OnUseAuth);
	m_reset.OnClick = Callback(this, &CFeedEditor::OnReset);
	m_help.OnClick = Callback(this, &CFeedEditor::OnHelp);

	if (feeds)
		SetParent(feeds->GetParent()->GetHwnd());
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
	g_plugin.setWord(hContact, "Status", g_nStatus);
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

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

INT_PTR AddFeed(WPARAM, LPARAM lParam)
{
	if (pAddFeedDialog == nullptr) {
		pAddFeedDialog = new CFeedEditor(-1, (CCtrlListView *)lParam, NULL);
		pAddFeedDialog->Show();
	}
	else {
		SetForegroundWindow(pAddFeedDialog->GetHwnd());
		SetFocus(pAddFeedDialog->GetHwnd());
	}
	return 0;
}

INT_PTR ChangeFeed(WPARAM hContact, LPARAM)
{
	CFeedEditor *pDlg = nullptr;
	for (auto &it : g_arFeeds)
		if (it->getContact() == hContact)
			pDlg = it;

	if (pDlg == nullptr) {
		pDlg = new CFeedEditor(-1, nullptr, hContact);
		pDlg->Show();
	}
	else {
		SetForegroundWindow(pDlg->GetHwnd());
		SetFocus(pDlg->GetHwnd());
	}
	return 0;
}

CDlgBase* FindFeedEditor(const wchar_t *pwszNick, const wchar_t *pwszUrl)
{
	for (auto &it : g_arFeeds) {
		ptrW dbNick(g_plugin.getWStringA(it->getContact(), "Nick"));
		if (dbNick == nullptr || mir_wstrcmp(dbNick, pwszNick))
			continue;

		ptrW dbURL(g_plugin.getWStringA(it->getContact(), "URL"));
		if (dbURL == nullptr || (mir_wstrcmp(dbURL, pwszUrl) != 0))
			continue;

		return it;
	}

	return nullptr;
}
