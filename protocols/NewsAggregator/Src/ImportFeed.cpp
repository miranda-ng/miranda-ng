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

static CDlgBase *pImportDialog = nullptr;

static const TiXmlElement* AdviceNode(const TiXmlElement *node)
{
	auto *tmpnode = node;

	// try to rest on the same level first
	node = node->NextSiblingElement();
	if (node)
		return node;

	do {
		// go up one level
		node = tmpnode->Parent()->ToElement();
		tmpnode = node;
		node = node->NextSiblingElement();
		if (node)
			return node;
	} while (mir_strcmpi(tmpnode->Name(), "body"));

	// nothing found or we reached body
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CImportFeed : public CDlgBase
{
	typedef CDlgBase CSuper;

	CCtrlListView *m_list;

	CCtrlEdit m_importfile;
	CCtrlButton m_browsefile;
	CCtrlListBox m_feedslist;
	CCtrlListBox m_feedsimportlist;
	CCtrlButton m_addfeed;
	CCtrlButton m_removefeed;
	CCtrlButton m_addallfeeds;
	CCtrlButton m_removeallfeeds;
	CCtrlButton m_ok;

public:
	CImportFeed(CCtrlListView *feeds) :
		CSuper(g_plugin, IDD_FEEDIMPORT),
		m_importfile(this, IDC_IMPORTFILEPATH), m_browsefile(this, IDC_BROWSEIMPORTFILE),
		m_feedslist(this, IDC_FEEDSLIST), m_feedsimportlist(this, IDC_FEEDSIMPORTLIST),
		m_addfeed(this, IDC_ADDFEED), m_removefeed(this, IDC_REMOVEFEED),
		m_addallfeeds(this, IDC_ADDALLFEEDS), m_removeallfeeds(this, IDC_REMOVEALLFEEDS),
		m_ok(this, IDOK)
	{
		m_list = feeds;
		m_browsefile.OnClick = Callback(this, &CImportFeed::OnBrowseFile);
		m_addfeed.OnClick = Callback(this, &CImportFeed::OnAddFeed);
		m_removefeed.OnClick = Callback(this, &CImportFeed::OnRemoveFeed);
		m_addallfeeds.OnClick = Callback(this, &CImportFeed::OnAddAllFeeds);
		m_removeallfeeds.OnClick = Callback(this, &CImportFeed::OnRemoveAllFeeds);

		m_feedslist.OnDblClick = Callback(this, &CImportFeed::OnFeedsList);
		m_feedsimportlist.OnDblClick = Callback(this, &CImportFeed::OnFeedsImportList);
	}

	bool OnInitDialog() override
	{
		Utils_RestoreWindowPositionNoSize(m_hwnd, NULL, MODULENAME, "ImportDlg");
		m_removefeed.Disable();
		m_removeallfeeds.Disable();
		m_ok.Disable();
		m_addfeed.Disable();
		m_addallfeeds.Disable();
		return true;
	}

	bool OnApply() override
	{
		wchar_t FileName[MAX_PATH];
		m_importfile.GetText(FileName, _countof(FileName));

		FILE *in = _wfopen(FileName, L"rb");
		if (in == nullptr)
			return false;

		TiXmlDocument doc;
		int res = doc.LoadFile(in);
		fclose(in);
		if (res != 0)
			return false;

		auto *node = TiXmlConst(&doc)["opml"]["body"]["outline"].ToElement();
		if (!node)
			node = TiXmlConst(&doc)["body"]["outline"].ToElement();
		if (node == nullptr)
			return false;

		int count = m_feedsimportlist.GetCount();
		int DUPES = 0;

		while (node) {
			auto *pszUrl = node->Attribute("xmlUrl");
			if (!pszUrl && node->NoChildren())
				node = AdviceNode(node);
			else if (!pszUrl && !node->NoChildren())
				node = node->FirstChildElement();
			else if (pszUrl) {
				wchar_t *text = nullptr, *url = nullptr, *siteurl = nullptr;
				bool bNeedToImport = false;

				if (auto *pszText = node->Attribute("text")) {
					text = mir_utf8decodeW(pszText);

					for (int j = 0; j < count; j++) {
						wchar_t item[MAX_PATH];
						m_feedsimportlist.GetItemText(j, item, _countof(item));
						if (!mir_wstrcmpi(item, text)) {
							bNeedToImport = true;
							break;
						}
					}
				}

				if (auto *pszText = node->Attribute("xmlUrl")) {
					url = mir_utf8decodeW(pszText);
					if (GetContactByURL(url) && bNeedToImport) {
						bNeedToImport = false;
						DUPES++;
					}
				}

				if (auto *pszText = node->Attribute("htmlUrl"))
					siteurl = mir_utf8decodeW(pszText);

				if (bNeedToImport && text && url) {
					CMStringW wszGroup;
					auto *parent = node->Parent()->ToElement();
					while (mir_strcmpi(parent->Name(), "body")) {
						if (auto *pszText = parent->Attribute("text")) {
							if (!wszGroup.IsEmpty())
								wszGroup.Insert(0, L"\\");
							wszGroup.Insert(0, Utf2T(pszText));
						}
						parent = parent->Parent()->ToElement();
					}

					MCONTACT hContact = db_add_contact();
					Proto_AddToContact(hContact, MODULENAME);
					Contact::Readonly(hContact);
					g_plugin.setWString(hContact, "Nick", text);
					g_plugin.setWString(hContact, "URL", url);
					if (siteurl)
						g_plugin.setWString(hContact, "Homepage", siteurl);
					g_plugin.setByte(hContact, "CheckState", 1);
					g_plugin.setDword(hContact, "UpdateTime", DEFAULT_UPDATE_TIME);
					g_plugin.setWString(hContact, "MsgFormat", TAGSDEFAULT);
					g_plugin.setWord(hContact, "Status", Proto_GetStatus(MODULENAME));

					if (m_list != nullptr) {
						int iItem = m_list->AddItem(text, -1);
						m_list->SetItem(iItem, 1, url);
						m_list->SetCheckState(iItem, 1);
					}

					if (!wszGroup.IsEmpty()) {
						Clist_GroupCreate(0, wszGroup);
						Clist_SetGroup(hContact, wszGroup);
					}
				}
				mir_free(text);
				mir_free(url);
				mir_free(siteurl);

				node = AdviceNode(node);
			}
		}

		wchar_t mes[MAX_PATH];
		if (DUPES)
			mir_snwprintf(mes, TranslateT("Imported %d feed(s)\r\nNot imported %d duplicate(s)."), count - DUPES, DUPES);
		else
			mir_snwprintf(mes, TranslateT("Imported %d feed(s)."), count);
		MessageBox(m_hwnd, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "ImportDlg");
		pImportDialog = nullptr;
	}

	void OnBrowseFile(CCtrlBase *)
	{
		wchar_t FileName[MAX_PATH];
		VARSW tszMirDir(L"%miranda_path%");

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		wchar_t tmp[MAX_PATH];
		mir_snwprintf(tmp, L"%s (*.opml, *.xml)%c*.opml;*.xml%c%c", TranslateT("OPML files"), 0, 0, 0);
		ofn.lpstrFilter = tmp;
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = FileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_HIDEREADONLY;
		ofn.lpstrInitialDir = tszMirDir;
		*FileName = '\0';
		ofn.lpstrDefExt = L"";
		if (!GetOpenFileName(&ofn))
			return;

		FILE *in = _wfopen(FileName, L"rb");
		if (in == nullptr)
			return;

		TiXmlDocument doc;
		int res = doc.LoadFile(in);
		fclose(in);
		if (res != 0) {
			MessageBox(m_hwnd, TranslateT("Not valid import file."), TranslateT("Error"), MB_OK | MB_ICONERROR);
			return;
		}

		m_importfile.SetText(FileName);

		auto *node = TiXmlConst(&doc)["opml"]["body"]["outline"].ToElement();
		if (!node)
			node = TiXmlConst(&doc)["body"]["outline"].ToElement();
		if (node == nullptr) {
			MessageBox(m_hwnd, TranslateT("Not valid import file."), TranslateT("Error"), MB_OK | MB_ICONERROR);
			return;
		}

		while (node) {
			auto *pszUrl = node->Attribute("xmlUrl");
			if (!pszUrl && node->NoChildren())
				node = AdviceNode(node);
			else if (!pszUrl && !node->NoChildren())
				node = node->FirstChildElement();
			else if (pszUrl) {
				if (auto *pszText = node->Attribute("text")) {
					Utf2T text(pszText);
					m_feedslist.AddString(text);
					m_addfeed.Enable();
					m_addallfeeds.Enable();
				}

				node = AdviceNode(node);
			}
		}
	}

	void OnAddFeed(CCtrlBase *)
	{
		if (!m_removefeed.Enabled())
			m_removefeed.Enable();
		if (!m_removeallfeeds.Enabled())
			m_removeallfeeds.Enable();
		if (!m_ok.Enabled())
			m_ok.Enable();
		int cursel = m_feedslist.GetCurSel();
		wchar_t item[MAX_PATH];
		m_feedslist.GetItemText(cursel, item, _countof(item));
		m_feedsimportlist.AddString(item);
		m_feedslist.DeleteString(cursel);
		if (!m_feedslist.GetCount()) {
			m_addfeed.Disable();
			m_addallfeeds.Disable();
		}
	}

	void OnRemoveFeed(CCtrlBase *)
	{
		if (!m_addfeed.Enabled())
			m_addfeed.Enable();
		if (!m_addallfeeds.Enabled())
			m_addallfeeds.Enable();
		int cursel = m_feedsimportlist.GetCurSel();
		wchar_t item[MAX_PATH];
		m_feedsimportlist.GetItemText(cursel, item, _countof(item));
		m_feedslist.AddString(item);
		m_feedsimportlist.DeleteString(cursel);
		if (!m_feedsimportlist.GetCount()) {
			m_removefeed.Disable();
			m_removeallfeeds.Disable();
			m_ok.Disable();
		}
	}

	void OnAddAllFeeds(CCtrlBase *)
	{
		if (!m_removefeed.Enabled())
			m_removefeed.Enable();
		if (!m_removeallfeeds.Enabled())
			m_removeallfeeds.Enable();
		if (!m_ok.Enabled())
			m_ok.Enable();
		int count = m_feedslist.GetCount();
		for (int i = 0; i < count; i++) {
			wchar_t item[MAX_PATH];
			m_feedslist.GetItemText(i, item, _countof(item));
			m_feedsimportlist.AddString(item);
		}
		for (int i = count - 1; i > -1; i--)
			m_feedslist.DeleteString(i);
		m_addfeed.Disable();
		m_addallfeeds.Disable();
	}

	void OnRemoveAllFeeds(CCtrlBase *)
	{
		if (!m_addfeed.Enabled())
			m_addfeed.Enable();
		if (!m_addallfeeds.Enabled())
			m_addallfeeds.Enable();
		int count = m_feedsimportlist.GetCount();
		for (int i = 0; i < count; i++) {
			wchar_t item[MAX_PATH];
			m_feedsimportlist.GetItemText(i, item, _countof(item));
			m_feedslist.AddString(item);
		}
		for (int i = count - 1; i > -1; i--)
			m_feedsimportlist.DeleteString(i);
		m_removefeed.Disable();
		m_removeallfeeds.Disable();
		m_ok.Disable();
	}

	void OnFeedsList(CCtrlBase *)
	{
		if (!m_removefeed.Enabled())
			m_removefeed.Enable();
		if (!m_removeallfeeds.Enabled())
			m_removeallfeeds.Enable();
		if (!m_ok.Enabled())
			m_ok.Enable();
		int cursel = m_feedslist.GetCurSel();
		wchar_t item[MAX_PATH];
		m_feedslist.GetItemText(cursel, item, _countof(item));
		m_feedsimportlist.AddString(item);
		m_feedslist.DeleteString(cursel);
		if (!m_feedslist.GetCount()) {
			m_addfeed.Disable();
			m_addallfeeds.Disable();
		}
	}

	void OnFeedsImportList(CCtrlBase *)
	{
		if (!m_addfeed.Enabled())
			m_addfeed.Enable();
		if (!m_addallfeeds.Enabled())
			m_addallfeeds.Enable();
		int cursel = m_feedsimportlist.GetCurSel();
		wchar_t item[MAX_PATH];
		m_feedsimportlist.GetItemText(cursel, item, _countof(item));
		m_feedslist.AddString(item);
		m_feedsimportlist.DeleteString(cursel);
		if (!m_feedsimportlist.GetCount()) {
			m_removefeed.Disable();
			m_removeallfeeds.Disable();
			m_ok.Disable();
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

INT_PTR ImportFeeds(WPARAM wParam, LPARAM)
{
	if (pImportDialog == nullptr) {
		pImportDialog = new CImportFeed(nullptr);
		if (wParam)
			pImportDialog->SetParent(MWindow(wParam));
	}
	pImportDialog->Show();
	return 0;
}
