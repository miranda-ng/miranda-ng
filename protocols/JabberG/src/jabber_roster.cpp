/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

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

#include <io.h>

//////////////////////////////////////////////////////////////////////////
// roster editor
//

enum
{
	RRA_FILLLIST = 0,
	RRA_SYNCROSTER,
	RRA_SYNCDONE
};

struct ROSTEREDITDAT
{
	HWND hList;
	int index;
	int subindex;
};

static void _RosterItemEditEnd(HWND hEditor, ROSTEREDITDAT *edat, BOOL bCancel)
{
	if (!bCancel) {
		int len = GetWindowTextLength(hEditor) + 1;
		wchar_t *buff = (wchar_t*)mir_alloc(len*sizeof(wchar_t));
		if (buff) {
			GetWindowText(hEditor, buff, len);
			ListView_SetItemText(edat->hList, edat->index, edat->subindex, buff);
		}
		mir_free(buff);
	}
	DestroyWindow(hEditor);
}

static LRESULT CALLBACK _RosterItemNewEditProc(HWND hEditor, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ROSTEREDITDAT * edat = (ROSTEREDITDAT *)GetWindowLongPtr(hEditor, GWLP_USERDATA);
	if (!edat) return 0;
	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			_RosterItemEditEnd(hEditor, edat, FALSE);
			return 0;
		case VK_ESCAPE:
			_RosterItemEditEnd(hEditor, edat, TRUE);
			return 0;
		}
		break;

	case WM_GETDLGCODE:
		if (lParam) {
			MSG *msg2 = (MSG*)lParam;
			if (msg2->message == WM_KEYDOWN && msg2->wParam == VK_TAB) return 0;
			if (msg2->message == WM_CHAR && msg2->wParam == '\t') return 0;
		}
		return DLGC_WANTMESSAGE;

	case WM_KILLFOCUS:
		_RosterItemEditEnd(hEditor, edat, FALSE);
		return 0;

	case WM_DESTROY:
		SetWindowLongPtr(hEditor, GWLP_USERDATA, (LONG_PTR)0);
		free(edat);
		return 0;
	}

	return mir_callNextSubclass(hEditor, _RosterItemNewEditProc, msg, wParam, lParam);
}

static LRESULT CALLBACK _RosterNewListProc(HWND hList, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_MOUSEWHEEL || msg == WM_NCLBUTTONDOWN || msg == WM_NCRBUTTONDOWN)
		SetFocus(hList);

	if (msg == WM_LBUTTONDOWN) {
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hList, &pt);

		LVHITTESTINFO lvhti = { 0 };
		lvhti.pt = pt;
		ListView_SubItemHitTest(hList, &lvhti);
		if (lvhti.flags&LVHT_ONITEM && lvhti.iSubItem != 0) {
			RECT rc;
			wchar_t buff[260];
			ListView_GetSubItemRect(hList, lvhti.iItem, lvhti.iSubItem, LVIR_BOUNDS, &rc);
			ListView_GetItemText(hList, lvhti.iItem, lvhti.iSubItem, buff, _countof(buff));
			HWND hEditor = CreateWindow(TEXT("EDIT"), buff, WS_CHILD | ES_AUTOHSCROLL, rc.left + 3, rc.top + 2, rc.right - rc.left - 3, rc.bottom - rc.top - 3, hList, nullptr, g_plugin.getInst(), nullptr);
			SendMessage(hEditor, WM_SETFONT, (WPARAM)SendMessage(hList, WM_GETFONT, 0, 0), 0);
			ShowWindow(hEditor, SW_SHOW);
			SetWindowText(hEditor, buff);
			ClientToScreen(hList, &pt);
			ScreenToClient(hEditor, &pt);
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

			ROSTEREDITDAT * edat = (ROSTEREDITDAT *)malloc(sizeof(ROSTEREDITDAT));
			mir_subclassWindow(hEditor, _RosterItemNewEditProc);
			edat->hList = hList;
			edat->index = lvhti.iItem;
			edat->subindex = lvhti.iSubItem;
			SetWindowLongPtr(hEditor, GWLP_USERDATA, (LONG_PTR)edat);
		}
	}
	return mir_callNextSubclass(hList, _RosterNewListProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberRosterOptDlgProc - advanced options dialog procedure

class CRosterEditorDlg : public CJabberDlgBase
{
	friend struct CJabberProto;
	typedef CJabberDlgBase CSuper;

	uint8_t m_bRRAction;
	BOOL m_bReadyToDownload = true;
	BOOL m_bReadyToUpload = false;

	void _RosterSendRequest(uint8_t rrAction)
	{
		m_bRRAction = rrAction;

		m_proto->m_ThreadInfo->send(
			XmlNodeIq(m_proto->AddIQ(&CJabberProto::_RosterHandleGetRequest, JABBER_IQ_TYPE_GET))
			<< XCHILDNS("query", JABBER_FEAT_IQ_ROSTER));
	}

	int _RosterInsertListItem(const char *jid, const char *nick, const char *group, const char *subscr, bool bChecked)
	{
		Utf2T wszJid(jid);
		LVITEM item = { 0 };
		item.mask = LVIF_TEXT | LVIF_STATE;
		item.iItem = m_list.GetItemCount();
		item.pszText = wszJid;

		int index = m_list.InsertItem(&item);
		if (index < 0)
			return index;

		m_list.SetCheckState(index, bChecked);

		m_list.SetItemText(index, 1, Utf2T(nick));
		m_list.SetItemText(index, 2, Utf2T(group));
		m_list.SetItemText(index, 3, TranslateW(Utf2T(subscr)));
		return index;
	}

	void _RosterListClear()
	{
		m_list.DeleteAllItems();
		while (m_list.GetColumnWidth(0) > 0)
			m_list.DeleteColumn(0);

		LV_COLUMN column = { 0 };
		column.mask = LVCF_TEXT;

		column.pszText = TranslateT("JID");
		m_list.InsertColumn(1, &column);

		column.pszText = TranslateT("Nickname");
		m_list.InsertColumn(2, &column);

		column.pszText = TranslateT("Group");
		m_list.InsertColumn(3, &column);

		column.pszText = TranslateT("Subscription");
		m_list.InsertColumn(4, &column);

		RECT rc;
		GetClientRect(m_list.GetHwnd(), &rc);
		ResizeColumns(rc.right - rc.left);
	}

	void ResizeColumns(int width)
	{
		m_list.SetColumnWidth(0, width * 40 / 100);
		m_list.SetColumnWidth(1, width * 25 / 100);
		m_list.SetColumnWidth(2, width * 20 / 100);
		m_list.SetColumnWidth(3, width * 12 / 100);
	}

	void OnChangeStatus()
	{
		int count = m_list.GetItemCount();
		btnDownload.Enable(m_proto->m_bJabberOnline);
		btnUpload.Enable(count && m_proto->m_bJabberOnline);
		btnExport.Enable(count > 0);
	}

	CCtrlButton btnDownload, btnUpload, btnExport, btnImport;
	CCtrlListView m_list;

public:
	CRosterEditorDlg(CJabberProto *m_proto) :
		CSuper(m_proto, IDD_ROSTER_EDITOR),
		m_list(this, IDC_ROSTER),
		btnExport(this, IDC_EXPORT),
		btnImport(this, IDC_IMPORT),
		btnUpload(this, IDC_UPLOAD),
		btnDownload(this, IDC_DOWNLOAD)
	{
		SetMinSize(550, 390);

		btnExport.OnClick = Callback(this, &CRosterEditorDlg::onClick_Export);
		btnImport.OnClick = Callback(this, &CRosterEditorDlg::onClick_Import);
		btnUpload.OnClick = Callback(this, &CRosterEditorDlg::onClick_Upload);
		btnDownload.OnClick = Callback(this, &CRosterEditorDlg::onClick_Download);
	}

	bool OnInitDialog() override
	{
		SetWindowTextW(m_hwnd, CMStringW(FORMAT, L"%s: %s", TranslateT("Roster Editor"), m_proto->m_tszUserName));

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_AGENTS));

		Utils_RestoreWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "rosterCtrlWnd_");

		m_list.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_BORDERSELECT | LVS_EX_GRIDLINES);
		mir_subclassWindow(m_list.GetHwnd(), _RosterNewListProc);
		_RosterListClear();
		OnChangeStatus();
		return true;
	}

	void OnDestroy() override
	{
		m_proto->m_hwndRosterEditor = nullptr;
		Utils_SaveWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "rosterCtrlWnd_");
		Window_FreeIcon_IcoLib(m_hwnd);
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_HEADERBAR:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH;
		case IDC_ROSTER:
			ResizeColumns(urc->rcItem.right - urc->rcItem.left + urc->dlgNewSize.cx - urc->dlgOriginalSize.cx);
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORY_HEIGHT | RD_ANCHORX_WIDTH;
		case IDC_DOWNLOAD:
		case IDC_UPLOAD:
			return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
		case IDC_EXPORT:
		case IDC_IMPORT:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	void HandleNode(const TiXmlElement *node)
	{
		if (m_bRRAction == RRA_FILLLIST) {
			_RosterListClear();
			auto *query = XmlFirstChild(node, "query");
			if (query == nullptr) return;

			for (auto *item : TiXmlFilter(query, "item")) {
				const char *jid = XmlGetAttr(item, "jid");
				if (jid == nullptr)
					continue;

				const char *name = XmlGetAttr(item, "name");
				const char *subscription = XmlGetAttr(item, "subscription");
				const char *group = XmlGetChildText(item, "group");
				_RosterInsertListItem(jid, name, group, subscription, true);
			}

			// now it is require to process whole contact list to add not in roster contacts
			for (auto &hContact : m_proto->AccContacts()) {
				ptrW tszJid(m_proto->getWStringA(hContact, "jid"));
				if (tszJid == nullptr)
					continue;

				LVFINDINFO lvfi = { 0 };
				lvfi.flags = LVFI_STRING;
				lvfi.psz = tszJid;
				if (m_list.FindItem(-1, &lvfi) == -1) {
					ptrA tszName(db_get_utfa(hContact, "CList", "MyHandle"));
					ptrA tszGroup(db_get_utfa(hContact, "CList", "Group"));
					_RosterInsertListItem(T2Utf(tszJid), tszName, tszGroup, nullptr, false);
				}
			}
			m_bReadyToDownload = false;
			m_bReadyToUpload = true;
			btnDownload.SetText(TranslateT("Download"));
			btnUpload.SetText(TranslateT("Upload"));
			OnChangeStatus();
			return;
		}

		if (m_bRRAction == RRA_SYNCROSTER) {
			btnUpload.SetText(TranslateT("Uploading..."));
			auto *queryRoster = XmlFirstChild(node, "query");
			if (!queryRoster)
				return;

			int ListItemCount = m_list.GetItemCount();
			for (int index = 0; index < ListItemCount; index++) {
				wchar_t jid[JABBER_MAX_JID_LEN] = L"";
				wchar_t name[260];
				wchar_t group[260];
				wchar_t subscr[260];
				m_list.GetItemText(index, 0, jid, _countof(jid));
				m_list.GetItemText(index, 1, name, _countof(name));
				m_list.GetItemText(index, 2, group, _countof(group));
				m_list.GetItemText(index, 3, subscr, _countof(subscr));

				T2Utf szJid(jid), szName(name), szGroup(group), szSubscr(subscr);
				auto *itemRoster = XmlGetChildByTag(queryRoster, "item", "jid", szJid);
				bool bRemove = !m_list.GetCheckState(index);
				if (itemRoster && bRemove) { // delete item
					XmlNodeIq iq(m_proto->AddIQ(&CJabberProto::_RosterHandleGetRequest, JABBER_IQ_TYPE_SET));
					iq << XCHILDNS("query", JABBER_FEAT_IQ_ROSTER) << XCHILD("item") << XATTR("jid", szJid) << XATTR("subscription", "remove");
					m_proto->m_ThreadInfo->send(iq);
				}
				else if (!bRemove) {
					bool bPushed = itemRoster != 0;
					const char *rosterName = 0, *rosterGroup = 0;
					if (!bPushed) {
						rosterName = XmlGetAttr(itemRoster, "name");
						if ((rosterName != nullptr || name[0] != 0) && mir_strcmpi(rosterName, szName))
							bPushed = true;
						if (!bPushed) {
							auto *szSub = XmlGetAttr(itemRoster, "subscription");
							if ((szSub != nullptr || subscr[0] != 0) && mir_strcmpi(szSub, szSubscr))
								bPushed = true;
						}
						if (!bPushed) {
							rosterGroup = XmlGetChildText(itemRoster, "group");
							if (rosterGroup != nullptr && mir_strcmpi(rosterGroup, szGroup))
								bPushed = true;
						}
					}
					if (bPushed) {
						XmlNodeIq iq(m_proto->AddIQ(&CJabberProto::_RosterHandleGetRequest, JABBER_IQ_TYPE_SET));
						auto *item = iq << XCHILDNS("query", JABBER_FEAT_IQ_ROSTER) << XCHILD("item");
						if (rosterGroup || mir_strlen(szGroup))
							item << XCHILD("group", szGroup);
						if (rosterName || mir_strlen(szName))
							item << XATTR("name", szName);
						item << XATTR("jid", szJid) << XATTR("subscription", subscr[0] ? szSubscr : "none");
						m_proto->m_ThreadInfo->send(iq);
					}
				}
			}
			m_bRRAction = RRA_SYNCDONE;
			_RosterSendRequest(RRA_FILLLIST);
			return;
		}

		btnUpload.SetText(TranslateT("Upload"));
		onClick_Download(nullptr);
	};

	void onClick_Download(CCtrlButton*)
	{
		m_bReadyToUpload = m_bReadyToDownload = false;
		OnChangeStatus();
		btnDownload.SetText(TranslateT("Downloading..."));
		_RosterSendRequest(RRA_FILLLIST);
	}

	void onClick_Upload(CCtrlButton*)
	{
		m_bReadyToUpload = false;
		OnChangeStatus();
		btnUpload.SetText(TranslateT("Connecting..."));
		_RosterSendRequest(RRA_SYNCROSTER);
	}

	void onClick_Export(CCtrlButton*)
	{
		wchar_t filename[MAX_PATH] = { 0 };

		wchar_t filter[MAX_PATH];
		mir_snwprintf(filter, L"%s (*.xml)%c*.xml%c%c", TranslateT("XML (UTF-8 encoded)"), 0, 0, 0);
		OPENFILENAME ofn = {};
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = m_hwnd;
		ofn.lpstrFilter = filter;
		ofn.lpstrFile = filename;
		ofn.Flags = OFN_HIDEREADONLY;
		ofn.nMaxFile = _countof(filename);
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.lpstrDefExt = L"xml";
		if (!GetSaveFileNameW(&ofn))
			return;

		FILE * fp = _wfopen(filename, L"wb");
		if (!fp)
			return;

		int ListItemCount = m_list.GetItemCount();

		XmlNode root("Roster");

		for (int index = 0; index < ListItemCount; index++) {
			wchar_t jid[JABBER_MAX_JID_LEN] = L"";
			wchar_t name[260] = L"";
			wchar_t group[260] = L"";
			wchar_t subscr[260] = L"";
			m_list.GetItemText(index, 0, jid, _countof(jid));
			m_list.GetItemText(index, 1, name, _countof(name));
			m_list.GetItemText(index, 2, group, _countof(group));
			m_list.GetItemText(index, 3, subscr, _countof(subscr));
			root << XCHILD("Item") << XATTR("jid", T2Utf(jid)) << XATTR("name", T2Utf(name)) << XATTR("group", T2Utf(group)) << XATTR("subscription", T2Utf(subscr));
		}

		tinyxml2::XMLPrinter printer(fp);
		root.Print(&printer);
		fclose(fp);
	}

	void onClick_Import(CCtrlButton*)
	{
		wchar_t filename[MAX_PATH] = {};

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = m_hwnd;
		ofn.hInstance = nullptr;
		ofn.lpstrFilter = L"XML (UTF-8 encoded)(*.xml)\0*.xml\0\0";
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrFile = filename;
		ofn.nMaxFile = _countof(filename);
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.lpstrDefExt = L"xml";
		if (!GetOpenFileNameW(&ofn))
			return;

		FILE * fp = _wfopen(filename, L"rb");
		if (!fp)
			return;

		TiXmlDocument doc;
		int ret = doc.LoadFile(fp);
		fclose(fp);
		if (ret != 0)
			return;

		_RosterListClear();

		const TiXmlElement *Table = TiXmlConst(&doc)["Workbook"]["Worksheet"]["Table"].ToElement();
		if (Table) {
			for (auto *Row : TiXmlFilter(Table, "Row")) {
				bool bAdd = false;
				const char *jid = nullptr;
				const char *name = nullptr;
				const char *group = nullptr;
				const char *subscr = nullptr;
				auto *Cell = XmlFirstChild(Row, "Cell");
				auto *Data = XmlFirstChild(Cell, "Data");
				if (Data) {
					if (!mir_strcmpi(Data->GetText(), "+"))
						bAdd = true;
					else if (mir_strcmpi(Data->GetText(), "-"))
						continue;

					Cell = Cell->NextSiblingElement("Cell");
					if (Cell) Data = XmlFirstChild(Cell, "Data");
					else Data = nullptr;
					if (Data) {
						jid = Data->GetText();
						if (!jid || mir_strlen(jid) == 0)
							continue;
					}

					Cell = Cell->NextSiblingElement("Cell");
					if (Cell) Data = XmlFirstChild(Cell, "Data");
					else Data = nullptr;
					if (Data) name = Data->GetText();

					Cell = Cell->NextSiblingElement("Cell");
					if (Cell) Data = XmlFirstChild(Cell, "Data");
					else Data = nullptr;
					if (Data) group = Data->GetText();

					Cell = Cell->NextSiblingElement("Cell");
					if (Cell) Data = XmlFirstChild(Cell, "Data");
					else Data = nullptr;
					if (Data) subscr = Data->GetText();
				}
				_RosterInsertListItem(jid, name, group, subscr, bAdd);
			}
		}
		else if (Table = TiXmlConst(&doc)["Roster"].ToElement()) {
			for (auto *Row : TiXmlFilter(Table, "Item")) {
				auto *jid = Row->Attribute("jid");
				auto *name = Row->Attribute("name");
				auto *group = Row->Attribute("group");
				auto *subscr = Row->Attribute("subscription");
				_RosterInsertListItem(jid, name, group, subscr, true);
			}
		}

		OnChangeStatus();
	}
};

INT_PTR __cdecl CJabberProto::OnMenuHandleRosterControl(WPARAM, LPARAM)
{
	if (m_hwndRosterEditor)
		SetForegroundWindow(m_hwndRosterEditor->GetHwnd());
	else {
		m_hwndRosterEditor = new CRosterEditorDlg(this);
		m_hwndRosterEditor->Show();
	}

	return 0;
}

void CJabberProto::_RosterHandleGetRequest(const TiXmlElement *node, CJabberIqInfo*)
{
	if (m_hwndRosterEditor)
		m_hwndRosterEditor->HandleNode(node);
}

void CJabberProto::JabberUpdateDialogs(BOOL)
{
	if (m_hwndRosterEditor)
		m_hwndRosterEditor->OnChangeStatus();
}
