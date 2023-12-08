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

static CDlgBase *pExportDialog = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////

class CExportFeed : public CDlgBase
{
	typedef CDlgBase CSuper;

	CCtrlListBox m_feedslist;
	CCtrlListBox m_feedsexportlist;
	CCtrlButton m_addfeed;
	CCtrlButton m_removefeed;
	CCtrlButton m_addallfeeds;
	CCtrlButton m_removeallfeeds;
	CCtrlButton m_ok;

public:
	CExportFeed() :
		CSuper(g_plugin, IDD_FEEDEXPORT),
		m_feedslist(this, IDC_FEEDSLIST), m_feedsexportlist(this, IDC_FEEDSEXPORTLIST),
		m_addfeed(this, IDC_ADDFEED), m_removefeed(this, IDC_REMOVEFEED),
		m_addallfeeds(this, IDC_ADDALLFEEDS), m_removeallfeeds(this, IDC_REMOVEALLFEEDS),
		m_ok(this, IDOK)
	{
		m_addfeed.OnClick = Callback(this, &CExportFeed::OnAddFeed);
		m_removefeed.OnClick = Callback(this, &CExportFeed::OnRemoveFeed);
		m_addallfeeds.OnClick = Callback(this, &CExportFeed::OnAddAllFeeds);
		m_removeallfeeds.OnClick = Callback(this, &CExportFeed::OnRemoveAllFeeds);

		m_feedslist.OnDblClick = Callback(this, &CExportFeed::OnFeedsList);
		m_feedsexportlist.OnDblClick = Callback(this, &CExportFeed::OnFeedsExportList);
	}

	bool OnInitDialog() override
	{
		Utils_RestoreWindowPositionNoSize(m_hwnd, NULL, MODULENAME, "ExportDlg");
		for (auto &hContact : Contacts(MODULENAME)) {
			ptrW message(g_plugin.getWStringA(hContact, "Nick"));
			if (message != nullptr)
				m_feedslist.AddString(message);
		}
		m_removefeed.Disable();
		m_removeallfeeds.Disable();
		m_ok.Disable();
		if (!m_feedslist.GetCount()) {
			m_addfeed.Disable();
			m_addallfeeds.Disable();
		}
		return true;
	}

	bool OnApply() override
	{
		wchar_t FileName[MAX_PATH];
		VARSW tszMirDir(L"%miranda_path%");

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		wchar_t tmp[MAX_PATH];
		mir_snwprintf(tmp, L"%s (*.opml)%c*.opml%c%c", TranslateT("OPML files"), 0, 0, 0);
		ofn.lpstrFilter = tmp;
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = FileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
		ofn.lpstrInitialDir = tszMirDir;
		*FileName = '\0';
		ofn.lpstrDefExt = L"";
		if (!GetSaveFileName(&ofn))
			return false;

		TiXmlDocument doc;
		auto *hXml = doc.NewElement("opml"); doc.InsertEndChild(hXml);
		hXml->SetAttribute("version", "1.0");

		auto *xmlHeader = doc.NewElement("head"); hXml->InsertEndChild(xmlHeader);
		auto *xmlTitle = doc.NewElement("title"); xmlTitle->SetText("Miranda NG NewsAggregator plugin export"); xmlHeader->InsertEndChild(xmlTitle);

		auto *xmlBody = doc.NewElement("body"); hXml->InsertEndChild(xmlBody);

		int count = m_feedsexportlist.GetCount();
		for (int i = 0; i < count; i++) {
			wchar_t item[MAX_PATH];
			m_feedsexportlist.GetItemText(i, item, _countof(item));
			MCONTACT hContact = GetContactByNick(item);
			wchar_t
				*title = g_plugin.getWStringA(hContact, "Nick"),
				*url = g_plugin.getWStringA(hContact, "URL"),
				*siteurl = g_plugin.getWStringA(hContact, "Homepage"),
				*group = Clist_GetGroup(hContact);

			TiXmlElement *elem = xmlBody;
			if (group) {
				wchar_t *section = wcstok(group, L"\\");
				while (section != nullptr) {
					TiXmlElement *existgroup = 0;
					for (auto *it : TiXmlFilter(elem, "outline")) {
						if (it->Attribute("title", T2Utf(section))) {
							existgroup = (TiXmlElement *)it;
							break;
						}
					}

					if (!existgroup) {
						auto *pNew = doc.NewElement("outline");
						pNew->SetAttribute("title", section); pNew->SetAttribute("text", section);
						elem->InsertEndChild(pNew);
						elem = pNew;
					}
					else elem = existgroup;

					section = wcstok(nullptr, L"\\");
				}
			}

			auto *pNew = doc.NewElement("outline"); elem->InsertEndChild(pNew);
			pNew->SetAttribute("text", title);
			pNew->SetAttribute("title", title);
			pNew->SetAttribute("type", "rss");
			pNew->SetAttribute("xmlUrl", url);
			pNew->SetAttribute("htmlUrl", siteurl);

			mir_free(title);
			mir_free(url);
			mir_free(siteurl);
			mir_free(group);
		}

		FILE *out = _wfopen(FileName, L"wb");
		if (out) {
			tinyxml2::XMLPrinter printer(out);
			doc.Print(&printer);
			fclose(out);
		}
		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "ExportDlg");
		pExportDialog = nullptr;
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
		m_feedsexportlist.AddString(item);
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
		int cursel = m_feedsexportlist.GetCurSel();
		wchar_t item[MAX_PATH];
		m_feedsexportlist.GetItemText(cursel, item, _countof(item));
		m_feedslist.AddString(item);
		m_feedsexportlist.DeleteString(cursel);
		if (!m_feedsexportlist.GetCount()) {
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
			m_feedsexportlist.AddString(item);
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
		int count = m_feedsexportlist.GetCount();
		for (int i = 0; i < count; i++) {
			wchar_t item[MAX_PATH];
			m_feedsexportlist.GetItemText(i, item, _countof(item));
			m_feedslist.AddString(item);
		}
		for (int i = count - 1; i > -1; i--)
			m_feedsexportlist.DeleteString(i);
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
		m_feedsexportlist.AddString(item);
		m_feedslist.DeleteString(cursel);
		if (!m_feedslist.GetCount()) {
			m_addfeed.Disable();
			m_addallfeeds.Disable();
		}
	}

	void OnFeedsExportList(CCtrlBase *)
	{
		if (!m_addfeed.Enabled())
			m_addfeed.Enable();
		if (!m_addallfeeds.Enabled())
			m_addallfeeds.Enable();
		int cursel = m_feedsexportlist.GetCurSel();
		wchar_t item[MAX_PATH];
		m_feedsexportlist.GetItemText(cursel, item, _countof(item));
		m_feedslist.AddString(item);
		m_feedsexportlist.DeleteString(cursel);
		if (!m_feedsexportlist.GetCount()) {
			m_removefeed.Disable();
			m_removeallfeeds.Disable();
			m_ok.Disable();
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

INT_PTR ExportFeeds(WPARAM wParam, LPARAM)
{
	if (pExportDialog == nullptr) {
		pExportDialog = new CExportFeed();
		if (wParam)
			pExportDialog->SetParent(MWindow(wParam));
	}
	pExportDialog->Show();
	return 0;
}
