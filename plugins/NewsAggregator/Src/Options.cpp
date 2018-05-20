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

CExportFeed::CExportFeed()
	: CSuper(g_plugin, IDD_FEEDEXPORT),
	m_feedslist(this, IDC_FEEDSLIST), m_feedsexportlist(this, IDC_FEEDSEXPORTLIST),
	m_addfeed(this, IDC_ADDFEED), m_removefeed(this, IDC_REMOVEFEED), 
	m_addallfeeds(this, IDC_ADDALLFEEDS), m_removeallfeeds(this, IDC_REMOVEALLFEEDS),
	m_ok(this, IDOK)
{
	m_addfeed.OnClick = Callback(this, &CExportFeed::OnAddFeed);
	m_removefeed.OnClick = Callback(this, &CExportFeed::OnRemoveFeed);
	m_addallfeeds.OnClick = Callback(this, &CExportFeed::OnAddAllFeeds);
	m_removeallfeeds.OnClick = Callback(this, &CExportFeed::OnRemoveAllFeeds);
	m_ok.OnClick = Callback(this, &CExportFeed::OnOk);

	m_feedslist.OnDblClick = Callback(this, &CExportFeed::OnFeedsList);
	m_feedsexportlist.OnDblClick = Callback(this, &CExportFeed::OnFeedsExportList);
}

void CExportFeed::OnInitDialog()
{
	Utils_RestoreWindowPositionNoSize(m_hwnd, NULL, MODULENAME, "ExportDlg");
	for (auto &hContact : Contacts(MODULENAME)) {
		wchar_t *message = db_get_wsa(hContact, MODULENAME, "Nick");
		if (message != nullptr) {
			m_feedslist.AddString(message);
			mir_free(message);
		}
	}
	m_removefeed.Disable();
	m_removeallfeeds.Disable();
	m_ok.Disable();
	if (!m_feedslist.GetCount()) {
		m_addfeed.Disable();
		m_addallfeeds.Disable();
	}
}

void CExportFeed::OnAddFeed(CCtrlBase*)
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

void CExportFeed::OnRemoveFeed(CCtrlBase*)
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

void CExportFeed::OnAddAllFeeds(CCtrlBase*)
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

void CExportFeed::OnRemoveAllFeeds(CCtrlBase*)
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

void CExportFeed::OnFeedsList(CCtrlBase*)
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

void CExportFeed::OnFeedsExportList(CCtrlBase*)
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

void CExportFeed::OnOk(CCtrlBase*)
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

	if (GetSaveFileName(&ofn)) {
		HXML hXml = xmlCreateNode(L"opml", nullptr, FALSE);
		xmlAddAttr(hXml, L"version", L"1.0");
		HXML header = xmlAddChild(hXml, L"head", nullptr);
		xmlAddChild(header, L"title", L"Miranda NG NewsAggregator plugin export");
		header = xmlAddChild(hXml, L"body", nullptr);

		int count = m_feedsexportlist.GetCount();
		for (int i = 0; i < count; i++) {
			wchar_t item[MAX_PATH];
			m_feedsexportlist.GetItemText(i, item, _countof(item));
			MCONTACT hContact = GetContactByNick(item);
			wchar_t
				*title = db_get_wsa(hContact, MODULENAME, "Nick"),
				*url = db_get_wsa(hContact, MODULENAME, "URL"),
				*siteurl = db_get_wsa(hContact, MODULENAME, "Homepage"),
				*group = db_get_wsa(hContact, "CList", "Group");

			HXML elem = header;
			if (group)
			{
				wchar_t *section = wcstok(group, L"\\");
				while (section != nullptr)
				{
					HXML existgroup = xmlGetChildByAttrValue(header, L"outline", L"title", section);
					if (!existgroup)
					{
						elem = xmlAddChild(elem, L"outline", nullptr);
						xmlAddAttr(elem, L"title", section);
						xmlAddAttr(elem, L"text", section);
					}
					else {
						elem = existgroup;
					}
					section = wcstok(nullptr, L"\\");
				}
				elem = xmlAddChild(elem, L"outline", nullptr);
			}
			else
				elem = xmlAddChild(elem, L"outline", nullptr);
			xmlAddAttr(elem, L"text", title);
			xmlAddAttr(elem, L"title", title);
			xmlAddAttr(elem, L"type", L"rss");
			xmlAddAttr(elem, L"xmlUrl", url);
			xmlAddAttr(elem, L"htmlUrl", siteurl);

			mir_free(title);
			mir_free(url);
			mir_free(siteurl);
			mir_free(group);
		}
		xmlToFile(hXml, FileName, 1);
		xmlDestroyNode(hXml);
	}
}

void CExportFeed::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "ExportDlg");
	if (pExportDialog)
		pExportDialog = nullptr;
}

CImportFeed::CImportFeed(CCtrlListView *m_feeds)
	: CSuper(g_plugin, IDD_FEEDIMPORT),
	m_importfile(this, IDC_IMPORTFILEPATH), m_browsefile(this, IDC_BROWSEIMPORTFILE),
	m_feedslist(this, IDC_FEEDSLIST), m_feedsimportlist(this, IDC_FEEDSIMPORTLIST),
	m_addfeed(this, IDC_ADDFEED), m_removefeed(this, IDC_REMOVEFEED),
	m_addallfeeds(this, IDC_ADDALLFEEDS), m_removeallfeeds(this, IDC_REMOVEALLFEEDS),
	m_ok(this, IDOK)
{
	m_list = m_feeds;
	m_browsefile.OnClick = Callback(this, &CImportFeed::OnBrowseFile);
	m_addfeed.OnClick = Callback(this, &CImportFeed::OnAddFeed);
	m_removefeed.OnClick = Callback(this, &CImportFeed::OnRemoveFeed);
	m_addallfeeds.OnClick = Callback(this, &CImportFeed::OnAddAllFeeds);
	m_removeallfeeds.OnClick = Callback(this, &CImportFeed::OnRemoveAllFeeds);
	m_ok.OnClick = Callback(this, &CImportFeed::OnOk);

	m_feedslist.OnDblClick = Callback(this, &CImportFeed::OnFeedsList);
	m_feedsimportlist.OnDblClick = Callback(this, &CImportFeed::OnFeedsImportList);
}

void CImportFeed::OnInitDialog()
{
	Utils_RestoreWindowPositionNoSize(m_hwnd, NULL, MODULENAME, "ImportDlg");
	m_removefeed.Disable();
	m_removeallfeeds.Disable();
	m_ok.Disable();
	m_addfeed.Disable();
	m_addallfeeds.Disable();
}

void CImportFeed::OnBrowseFile(CCtrlBase*)
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

	if (GetOpenFileName(&ofn)) {
		int bytesParsed = 0;
		HXML hXml = xmlParseFile(FileName, &bytesParsed, nullptr);
		if (hXml != nullptr) {
			HXML node = xmlGetChildByPath(hXml, L"opml/body/outline", 0);
			if (!node)
				node = xmlGetChildByPath(hXml, L"body/outline", 0);
			if (node) {
				while (node) {
					int outlineAttr = xmlGetAttrCount(node);
					int outlineChildsCount = xmlGetChildCount(node);
					wchar_t *xmlUrl = (wchar_t *)xmlGetAttrValue(node, L"xmlUrl");
					if (!xmlUrl && !outlineChildsCount) {
						HXML tmpnode = node;
						node = xmlGetNextNode(node);
						if (!node) {
							do {
								node = tmpnode;
								node = xmlGetParent(node);
								tmpnode = node;
								node = xmlGetNextNode(node);
								if (node)
									break;
							} while (mir_wstrcmpi(xmlGetName(node), L"body"));
						}
					}
					else if (!xmlUrl && outlineChildsCount)
						node = xmlGetFirstChild(node);
					else if (xmlUrl) {
						for (int i = 0; i < outlineAttr; i++) {
							if (!mir_wstrcmpi(xmlGetAttrName(node, i), L"text")) {
								wchar_t *text = mir_utf8decodeW(_T2A(xmlGetAttrValue(node, xmlGetAttrName(node, i))));
								bool isTextUTF;
								if (!text) {
									isTextUTF = false;
									text = (wchar_t *)xmlGetAttrValue(node, xmlGetAttrName(node, i));
								}
								else
									isTextUTF = true;
								m_feedslist.AddString(text);
								m_addfeed.Enable();
								m_addallfeeds.Enable();
								if (isTextUTF)
									mir_free(text);
							}
						}

						HXML tmpnode = node;
						node = xmlGetNextNode(node);
						if (!node) {
							do {
								node = tmpnode;
								node = xmlGetParent(node);
								tmpnode = node;
								node = xmlGetNextNode(node);
								if (node)
									break;
							} while (mir_wstrcmpi(xmlGetName(tmpnode), L"body"));
						}
					}
				}
			}
			else
				MessageBox(m_hwnd, TranslateT("Not valid import file."), TranslateT("Error"), MB_OK | MB_ICONERROR);
			xmlDestroyNode(hXml);
			m_importfile.SetText(FileName);
		}
		else
			MessageBox(m_hwnd, TranslateT("Not valid import file."), TranslateT("Error"), MB_OK | MB_ICONERROR);
	}
}

void CImportFeed::OnAddFeed(CCtrlBase*)
{
	if (!m_removefeed.Enabled())
		m_removefeed.Enable();
	if (!m_removeallfeeds.Enabled())
		m_removeallfeeds.Enable();
	if (!m_ok.Enabled())
		m_ok.Enable();
	int cursel =  m_feedslist.GetCurSel();
	wchar_t item[MAX_PATH];
	m_feedslist.GetItemText(cursel, item, _countof(item));
	m_feedsimportlist.AddString(item);
	m_feedslist.DeleteString(cursel);
	if (!m_feedslist.GetCount()) {
		m_addfeed.Disable();
		m_addallfeeds.Disable();
	}
}

void CImportFeed::OnRemoveFeed(CCtrlBase*)
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

void CImportFeed::OnAddAllFeeds(CCtrlBase*)
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

void CImportFeed::OnRemoveAllFeeds(CCtrlBase*)
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

void CImportFeed::OnFeedsList(CCtrlBase*)
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

void CImportFeed::OnFeedsImportList(CCtrlBase*)
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

void CImportFeed::OnOk(CCtrlBase*)
{
	wchar_t FileName[MAX_PATH];
	m_importfile.GetText(FileName, _countof(FileName));
	int bytesParsed = 0;
	HXML hXml = xmlParseFile(FileName, &bytesParsed, nullptr);
	if (hXml != nullptr) {
		bool isTextUTF = false, isURLUTF = false, isSiteURLUTF = false, isGroupUTF = false;
		HXML node = xmlGetChildByPath(hXml, L"opml/body/outline", 0);
		if (!node)
			node = xmlGetChildByPath(hXml, L"body/outline", 0);
		int count = m_feedsimportlist.GetCount();
		int DUPES = 0;
		if (node) {
			while (node) {
				int outlineAttr = xmlGetAttrCount(node);
				int outlineChildsCount = xmlGetChildCount(node);
				wchar_t *xmlUrl = (wchar_t *)xmlGetAttrValue(node, L"xmlUrl");
				if (!xmlUrl && !outlineChildsCount) {
					HXML tmpnode = node;
					node = xmlGetNextNode(node);
					if (!node) {
						do {
							node = tmpnode;
							node = xmlGetParent(node);
							tmpnode = node;
							node = xmlGetNextNode(node);
							if (node)
								break;
						} while (mir_wstrcmpi(xmlGetName(node), L"body"));
					}
				}
				else if (!xmlUrl && outlineChildsCount)
					node = xmlGetFirstChild(node);
				else if (xmlUrl) {
					wchar_t *text = nullptr, *url = nullptr, *siteurl = nullptr, *group = nullptr;
					BYTE NeedToImport = FALSE;
					for (int i = 0; i < outlineAttr; i++) {
						if (!mir_wstrcmpi(xmlGetAttrName(node, i), L"text")) {
							text = mir_utf8decodeW(_T2A(xmlGetAttrValue(node, xmlGetAttrName(node, i))));
							if (!text) {
								isTextUTF = 0;
								text = (wchar_t *)xmlGetAttrValue(node, xmlGetAttrName(node, i));
							}
							else
								isTextUTF = 1;

							for (int j = 0; j < count; j++) {
								wchar_t item[MAX_PATH];
								m_feedsimportlist.GetItemText(j, item, _countof(item));
								if (!mir_wstrcmpi(item, text)) {
									NeedToImport = TRUE;
									break;
								}
							}
							continue;
						}
						if (!mir_wstrcmpi(xmlGetAttrName(node, i), L"xmlUrl")) {
							url = mir_utf8decodeW(_T2A(xmlGetAttrValue(node, xmlGetAttrName(node, i))));
							if (!url) {
								isURLUTF = false;
								url = (wchar_t *)xmlGetAttrValue(node, xmlGetAttrName(node, i));
							}
							else
								isURLUTF = true;
							if (GetContactByURL(url) && NeedToImport) {
								NeedToImport = FALSE;
								DUPES++;
							}
							continue;
						}
						if (!mir_wstrcmpi(xmlGetAttrName(node, i), L"htmlUrl")) {
							siteurl = mir_utf8decodeW(_T2A(xmlGetAttrValue(node, xmlGetAttrName(node, i))));
							if (!siteurl) {
								isSiteURLUTF = false;
								siteurl = (wchar_t *)xmlGetAttrValue(node, xmlGetAttrName(node, i));
							}
							else
								isSiteURLUTF = true;
							continue;
						}
						if (text && url && siteurl)
							break;
					}

					if (NeedToImport) {
						HXML parent = xmlGetParent(node);
						wchar_t tmpgroup[1024];
						while (mir_wstrcmpi(xmlGetName(parent), L"body")) {
							for (int i = 0; i < xmlGetAttrCount(parent); i++) {
								if (!mir_wstrcmpi(xmlGetAttrName(parent, i), L"text")) {
									if (!group)
										group = (wchar_t *)xmlGetAttrValue(parent, xmlGetAttrName(parent, i));
									else {
										mir_snwprintf(tmpgroup, L"%s\\%s", xmlGetAttrValue(parent, xmlGetAttrName(parent, i)), group);
										group = tmpgroup;
									}
									break;
								}
							}
							parent = xmlGetParent(parent);
						}

						wchar_t *ptszGroup = nullptr;
						if (group) {
							ptszGroup = mir_utf8decodeW(_T2A(group));
							if (!ptszGroup) {
								isGroupUTF = false;
								ptszGroup = group;
							}
							else
								isGroupUTF = 1;
						}

						MCONTACT hContact = db_add_contact();
						Proto_AddToContact(hContact, MODULENAME);
						db_set_ws(hContact, MODULENAME, "Nick", text);
						db_set_ws(hContact, MODULENAME, "URL", url);
						db_set_ws(hContact, MODULENAME, "Homepage", siteurl);
						db_set_b(hContact, MODULENAME, "CheckState", 1);
						db_set_dw(hContact, MODULENAME, "UpdateTime", DEFAULT_UPDATE_TIME);
						db_set_ws(hContact, MODULENAME, "MsgFormat", TAGSDEFAULT);
						db_set_w(hContact, MODULENAME, "Status", Proto_GetStatus(MODULENAME));

						if (m_list != nullptr) {
							int iItem = m_list->AddItem(text, -1);
							m_list->SetItem(iItem, 1, url);
							m_list->SetCheckState(iItem, 1);
						}

						if (ptszGroup) {
							db_set_ws(hContact, "CList", "Group", ptszGroup);
							Clist_GroupCreate(0, ptszGroup);
						}
						if (isGroupUTF)
							mir_free(ptszGroup);
					}
					if (isTextUTF)
						mir_free(text);
					if (isURLUTF)
						mir_free(url);
					if (isSiteURLUTF)
						mir_free(siteurl);

					HXML tmpnode = node;
					node = xmlGetNextNode(node);
					if (!node) {
						do {
							node = tmpnode;
							node = xmlGetParent(node);
							tmpnode = node;
							node = xmlGetNextNode(node);
							if (node)
								break;
						} while (mir_wstrcmpi(xmlGetName(tmpnode), L"body"));
					}
				}
			}
		}
		xmlDestroyNode(hXml);
		wchar_t mes[MAX_PATH];
		if (DUPES)
			mir_snwprintf(mes, TranslateT("Imported %d feed(s)\r\nNot imported %d duplicate(s)."), count - DUPES, DUPES);
		else
			mir_snwprintf(mes, TranslateT("Imported %d feed(s)."), count);
		MessageBox(m_hwnd, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
	}
}

void CImportFeed::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "ImportDlg");
	if (pImportDialog)
		pImportDialog = nullptr;
}

CFeedEditor::CFeedEditor(int iItem, CCtrlListView *m_feeds, MCONTACT Contact)
	: CSuper(g_plugin, IDD_ADDFEED),
	m_feedtitle(this, IDC_FEEDTITLE), m_feedurl(this, IDC_FEEDURL),
	m_checktime(this, IDC_CHECKTIME), m_checktimespin(this, IDC_TIMEOUT_VALUE_SPIN),
	m_checkfeed(this, IDC_DISCOVERY), m_useauth(this, IDC_USEAUTH),
	m_login(this, IDC_LOGIN), m_password(this, IDC_PASSWORD),
	m_tagedit(this, IDC_TAGSEDIT), m_reset(this, IDC_RESET),
	m_help(this, IDC_TAGHELP), m_ok(this, IDOK), m_iItem(iItem)
{
	m_list = m_feeds;
	m_hContact = Contact;
	m_checkfeed.OnClick = Callback(this, &CFeedEditor::OnCheckFeed);
	m_useauth.OnChange = Callback(this, &CFeedEditor::OnUseAuth);
	m_reset.OnClick = Callback(this, &CFeedEditor::OnReset);
	m_help.OnClick = Callback(this, &CFeedEditor::OnHelp);
	m_ok.OnClick = Callback(this, &CFeedEditor::OnOk);
}

void CFeedEditor::OnInitDialog()
{
	if (m_iItem == -1 && m_hContact == NULL)
		SetWindowText(m_hwnd, TranslateT("Add Feed"));
	else
		SetWindowText(m_hwnd, TranslateT("Change Feed"));
	m_checktime.SetMaxLength(3);
	m_checktimespin.SetRange(999, 0);

	if (m_iItem > -1 && m_hContact == 0) {
		wchar_t SelNick[MAX_PATH], SelUrl[MAX_PACKAGE_NAME];
		m_list->GetItemText(m_iItem, 0, SelNick, _countof(SelNick));
		m_list->GetItemText(m_iItem, 1, SelUrl, _countof(SelNick));

		for (auto &hContact : Contacts(MODULENAME)) {
			ptrW dbNick(db_get_wsa(hContact, MODULENAME, "Nick"));
			if ((dbNick == NULL) || (mir_wstrcmp(dbNick, SelNick) != 0))
				continue;

			ptrW dbURL(db_get_wsa(hContact, MODULENAME, "URL"));
			if ((dbURL == NULL) || (mir_wstrcmp(dbURL, SelUrl) != 0))
				continue;

			m_hContact = hContact;
			m_feedtitle.SetText(SelNick);
			m_feedurl.SetText(SelUrl);
			m_checktime.SetInt(db_get_dw(hContact, MODULENAME, "UpdateTime", DEFAULT_UPDATE_TIME));

			wchar_t *szMsgFormat = db_get_wsa(hContact, MODULENAME, "MsgFormat");
			if (szMsgFormat) {
				m_tagedit.SetText(szMsgFormat);
				mir_free(szMsgFormat);
			}
			if (db_get_b(hContact, MODULENAME, "UseAuth", 0)) {
				m_useauth.SetState(1);
				m_login.Enable();
				m_password.Enable();

				wchar_t *szLogin = db_get_wsa(hContact, MODULENAME, "Login");
				if (szLogin) {
					m_login.SetText(szLogin);
					mir_free(szLogin);
				}
				pass_ptrA pwd(db_get_sa(hContact, MODULENAME, "Password"));
				m_password.SetTextA(pwd);
			}
			g_arFeeds.insert(this);
			Utils_RestoreWindowPositionNoSize(m_hwnd, hContact, MODULENAME, "ChangeDlg");
			break;
		}
	}
	else if (m_iItem == -1 && m_hContact == NULL) {
		m_feedurl.SetText(L"http://");
		m_tagedit.SetText(TAGSDEFAULT);
		m_checktime.SetInt(DEFAULT_UPDATE_TIME);
		Utils_RestoreWindowPositionNoSize(m_hwnd, NULL, MODULENAME, "AddDlg");
	}
	else if (m_hContact != NULL) {
		ptrW dbNick(db_get_wsa(m_hContact, MODULENAME, "Nick"));
		ptrW dbURL(db_get_wsa(m_hContact, MODULENAME, "URL"));

		m_feedtitle.SetText(dbNick);
		m_feedurl.SetText(dbURL);
		m_checktime.SetInt(db_get_dw(m_hContact, MODULENAME, "UpdateTime", DEFAULT_UPDATE_TIME));

		wchar_t *szMsgFormat = db_get_wsa(m_hContact, MODULENAME, "MsgFormat");
		if (szMsgFormat) {
			m_tagedit.SetText(szMsgFormat);
			mir_free(szMsgFormat);
		}
		if (db_get_b(m_hContact, MODULENAME, "UseAuth", 0)) {
			m_useauth.SetState(1);
			m_login.Enable();
			m_password.Enable();

			wchar_t *szLogin = db_get_wsa(m_hContact, MODULENAME, "Login");
			if (szLogin) {
				m_login.SetText(szLogin);
				mir_free(szLogin);
			}
			pass_ptrA pwd(db_get_sa(m_hContact, MODULENAME, "Password"));
			m_password.SetTextA(pwd);
		}
		g_arFeeds.insert(this);
		Utils_RestoreWindowPositionNoSize(m_hwnd, m_hContact, MODULENAME, "ChangeDlg");
	}
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

void CFeedEditor::OnOk(CCtrlBase*)
{
	ptrW strfeedtitle(m_feedtitle.GetText());
	if (!strfeedtitle || mir_wstrcmp(strfeedtitle, L"") == 0) {
		MessageBox(m_hwnd, TranslateT("Enter Feed name"), TranslateT("Error"), MB_OK);
		return;
	}

	ptrW strfeedurl(m_feedurl.GetText());
	if (!strfeedurl || mir_wstrcmp(strfeedurl, L"http://") == 0 || mir_wstrcmp(strfeedurl, L"") == 0) {
		MessageBox(m_hwnd, TranslateT("Enter Feed URL"), TranslateT("Error"), MB_OK);
		return;
	}

	ptrW strtagedit(m_tagedit.GetText());
	if (!strtagedit || mir_wstrcmp(strtagedit, L"") == 0) {
		MessageBox(m_hwnd, TranslateT("Enter message format"), TranslateT("Error"), MB_OK);
		return;
	}

	MCONTACT hContact;
	if (m_iItem == -1 && m_hContact == NULL) {
		hContact = db_add_contact();
		Proto_AddToContact(hContact, MODULENAME);
		db_set_b(hContact, MODULENAME, "CheckState", 1);
	}
	else
		hContact = m_hContact;

	db_set_ws(hContact, MODULENAME, "Nick", strfeedtitle);
	db_set_ws(hContact, MODULENAME, "URL", strfeedurl);
	db_set_dw(hContact, MODULENAME, "UpdateTime", (DWORD)m_checktime.GetInt());
	db_set_ws(hContact, MODULENAME, "MsgFormat", strtagedit);
	db_set_w(hContact, MODULENAME, "Status", Proto_GetStatus(MODULENAME));
	if (m_useauth.IsChecked()) {
		db_set_b(hContact, MODULENAME, "UseAuth", 1);
		db_set_ws(hContact, MODULENAME, "Login", m_login.GetText());
		db_set_s(hContact, MODULENAME, "Password", m_password.GetTextA());
	}
	else {
		db_unset(hContact, MODULENAME, "UseAuth");
		db_unset(hContact, MODULENAME, "Login");
		db_unset(hContact, MODULENAME, "Password");
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
}

void CFeedEditor::OnClose()
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
		wchar_t *ptszNick = db_get_wsa(hContact, MODULENAME, "Nick");
		if (ptszNick) {
			int iItem = m_feeds.AddItem(ptszNick, -1);

			wchar_t *ptszURL = db_get_wsa(hContact, MODULENAME, "URL");
			if (ptszURL) {
				m_feeds.SetItem(iItem, 1, ptszURL);
				m_feeds.SetCheckState(iItem, db_get_b(hContact, MODULENAME, "CheckState", 1));
				mir_free(ptszURL);
			}
			mir_free(ptszNick);
		}
	}
	UpdateListFlag = FALSE;
}

COptionsMain::COptionsMain()
	: CPluginDlgBase(g_plugin, IDD_OPTIONS, MODULENAME),
	m_feeds(this, IDC_FEEDLIST),
	m_add(this, IDC_ADD),
	m_change(this, IDC_CHANGE),
	m_delete(this, IDC_REMOVE),
	m_import(this, IDC_IMPORT),
	m_export(this, IDC_EXPORT),
	m_checkonstartup(this, IDC_STARTUPRETRIEVE)
{
	CreateLink(m_checkonstartup, "AutoUpdate", DBVT_BYTE, 1);

	m_add.OnClick = Callback(this, &COptionsMain::OnAddButtonClick);
	m_change.OnClick = Callback(this, &COptionsMain::OnChangeButtonClick);
	m_delete.OnClick = Callback(this, &COptionsMain::OnDeleteButtonClick);
	m_import.OnClick = Callback(this, &COptionsMain::OnImportButtonClick);
	m_export.OnClick = Callback(this, &COptionsMain::OnExportButtonClick);

	m_feeds.OnItemChanged = Callback(this, &COptionsMain::OnFeedListItemChanged);
	m_feeds.OnDoubleClick = Callback(this, &COptionsMain::OnFeedListDoubleClick);

}

void COptionsMain::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	m_change.Disable();
	m_delete.Disable();
	m_feeds.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	m_feeds.AddColumn(0, TranslateT("Feed"), 160);
	m_feeds.AddColumn(1, TranslateT("URL"), 276);
	UpdateList();
}

void COptionsMain::OnApply()
{
	for (auto &hContact : Contacts(MODULENAME)) {
		ptrW dbNick(db_get_wsa(hContact, MODULENAME, "Nick"));
		for (int i = 0; i < m_feeds.GetItemCount(); i++) {
			wchar_t nick[MAX_PATH];
			m_feeds.GetItemText(i, 0, nick, _countof(nick));
			if (mir_wstrcmp(dbNick, nick) == 0)
			{
				db_set_b(hContact, MODULENAME, "CheckState", m_feeds.GetCheckState(i));
				if (!m_feeds.GetCheckState(i))
					db_set_b(hContact, "CList", "Hidden", 1);
				else
					db_unset(hContact, "CList", "Hidden");
			}
		}
	}
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
	for (auto &it : g_arFeeds)
	{
		wchar_t nick[MAX_PATH], url[MAX_PATH];
		m_feeds.GetItemText(isel, 0, nick, _countof(nick));
		m_feeds.GetItemText(isel, 1, url, _countof(url));

		ptrW dbNick(db_get_wsa(it->getContact(), MODULENAME, "Nick"));
		if ((dbNick == NULL) || (mir_wstrcmp(dbNick, nick) != 0))
			continue;

		ptrW dbURL(db_get_wsa(it->getContact(), MODULENAME, "URL"));
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
			ptrW dbNick(db_get_wsa(hContact, MODULENAME, "Nick"));
			if (dbNick == NULL)
				break;
			if (mir_wstrcmp(dbNick, nick))
				continue;

			ptrW dbURL(db_get_wsa(hContact, MODULENAME, "URL"));
			if (dbURL == NULL)
				break;
			if (mir_wstrcmp(dbURL, url))
				continue;

			db_delete_contact(hContact);
			m_feeds.DeleteItem(isel);
			break;
		}
	}
}

void COptionsMain::OnImportButtonClick(CCtrlBase*)
{
	if (pImportDialog == nullptr) {
		pImportDialog = new CImportFeed(&m_feeds);
		pImportDialog->Show();
		pImportDialog->SetParent(m_hwnd);
	}
}

void COptionsMain::OnExportButtonClick(CCtrlBase*)
{
	if (pExportDialog == nullptr) {
		pExportDialog = new CExportFeed();
		pExportDialog->Show();
		pExportDialog->SetParent(m_hwnd);
	}
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
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_plugin.getInst();
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = LPGENW("News Aggregator");
	odp.pDialog = new COptionsMain();
	Options_AddPage(wParam, &odp);
	return 0;
}
