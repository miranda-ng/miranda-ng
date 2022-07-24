/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2007  Michael Stepura, George Hazan
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
#include "jabber_iq.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Bookmarks editor window

struct JabberAddBookmarkDlgParam
{
	CJabberProto *ppro;
	JABBER_LIST_ITEM* m_item;
};

static INT_PTR CALLBACK JabberAddBookmarkDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JabberAddBookmarkDlgParam* param = (JabberAddBookmarkDlgParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	wchar_t text[512];
	JABBER_LIST_ITEM *item;

	switch (msg) {
	case WM_INITDIALOG:
		param = (JabberAddBookmarkDlgParam*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		param->ppro->m_hwndJabberAddBookmark = hwndDlg;
		TranslateDialogDefault(hwndDlg);
		if (item = param->m_item) {
			if (!mir_strcmp(item->type, "conference")) {
				if (!strchr(item->jid, '@')) {	  //no room name - consider it is transport
					CheckDlgButton(hwndDlg, IDC_AGENT_RADIO, BST_CHECKED);
					EnableWindow(GetDlgItem(hwndDlg, IDC_NICK), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), FALSE);
				}
				else CheckDlgButton(hwndDlg, IDC_ROOM_RADIO, BST_CHECKED);
			}
			else {
				CheckDlgButton(hwndDlg, IDC_URL_RADIO, BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_NICK), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), FALSE);
				CheckDlgButton(hwndDlg, IDC_CHECK_BM_AUTOJOIN, BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_BM_AUTOJOIN), FALSE);
			}

			EnableWindow(GetDlgItem(hwndDlg, IDC_ROOM_RADIO), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_URL_RADIO), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AGENT_RADIO), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_BM_AUTOJOIN), FALSE);

			if (item->jid) SetDlgItemTextUtf(hwndDlg, IDC_ROOM_JID, item->jid);
			if (item->name) SetDlgItemTextW(hwndDlg, IDC_NAME, item->name);
			if (item->nick) SetDlgItemTextUtf(hwndDlg, IDC_NICK, item->nick);
			if (item->password) SetDlgItemTextUtf(hwndDlg, IDC_PASSWORD, item->password);
			if (item->bAutoJoin) CheckDlgButton(hwndDlg, IDC_CHECK_BM_AUTOJOIN, BST_CHECKED);
			if (IsDlgButtonChecked(hwndDlg, IDC_ROOM_RADIO) == BST_CHECKED)
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_BM_AUTOJOIN), TRUE);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
			CheckDlgButton(hwndDlg, IDC_ROOM_RADIO, BST_CHECKED);
		}
		return TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDC_ROOM_RADIO:
				EnableWindow(GetDlgItem(hwndDlg, IDC_NICK), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_BM_AUTOJOIN), TRUE);
				break;
			case IDC_AGENT_RADIO:
			case IDC_URL_RADIO:
				EnableWindow(GetDlgItem(hwndDlg, IDC_NICK), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), FALSE);
				CheckDlgButton(hwndDlg, IDC_CHECK_BM_AUTOJOIN, BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_BM_AUTOJOIN), FALSE);
				break;
			}
		}

		switch (LOWORD(wParam)) {
		case IDC_ROOM_JID:
			if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE)
				EnableWindow(GetDlgItem(hwndDlg, IDOK), GetDlgItemText(hwndDlg, IDC_ROOM_JID, text, _countof(text)));
			break;

		case IDOK:
			{
				GetDlgItemText(hwndDlg, IDC_ROOM_JID, text, _countof(text));
				T2Utf roomJID(text);

				if (param->m_item)
					param->ppro->ListRemove(LIST_BOOKMARK, param->m_item->jid);

				item = param->ppro->ListAdd(LIST_BOOKMARK, roomJID);

				if (IsDlgButtonChecked(hwndDlg, IDC_URL_RADIO) == BST_CHECKED)
					replaceStr(item->type, "url");
				else
					replaceStr(item->type, "conference");

				GetDlgItemText(hwndDlg, IDC_NICK, text, _countof(text));
				replaceStr(item->nick, T2Utf(text));

				GetDlgItemText(hwndDlg, IDC_PASSWORD, text, _countof(text));
				replaceStr(item->password, T2Utf(text));

				GetDlgItemText(hwndDlg, IDC_NAME, text, _countof(text));
				replaceStrW(item->name, (text[0] == 0) ? Utf2T(roomJID) : text);

				item->bAutoJoin = IsDlgButtonChecked(hwndDlg, IDC_CHECK_BM_AUTOJOIN) == BST_CHECKED;

				XmlNodeIq iq(param->ppro->AddIQ(&CJabberProto::OnIqResultSetBookmarks, JABBER_IQ_TYPE_SET));
				param->ppro->SetBookmarkRequest(iq);
				param->ppro->m_ThreadInfo->send(iq);
			}
			__fallthrough;

		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
		}
		break;

	case WM_PROTO_CHECK_ONLINE:
		if (!param->ppro->m_bJabberOnline)
			EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		param->ppro->m_hwndJabberAddBookmark = nullptr;
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Bookmarks manager window

class CJabberDlgBookmarks : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlMButton m_btnAdd, m_btnEdit, m_btnRemove;
	CCtrlListView m_lvBookmarks;

public:
	CJabberDlgBookmarks(CJabberProto *proto) :
		CSuper(proto, IDD_BOOKMARKS),
		m_btnAdd(this, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add")),
		m_btnEdit(this, IDC_EDIT, SKINICON_OTHER_EDIT, LPGEN("Edit")),
		m_btnRemove(this, IDC_REMOVE, SKINICON_OTHER_DELETE, LPGEN("Remove")),
		m_lvBookmarks(this, IDC_BM_LIST)
	{
		SetMinSize(451, 320);

		m_lvBookmarks.OnDoubleClick = Callback(this, &CJabberDlgBookmarks::lvBookmarks_OnDoubleClick);

		m_btnAdd.OnClick = Callback(this, &CJabberDlgBookmarks::btnAdd_OnClick);
		m_btnEdit.OnClick = Callback(this, &CJabberDlgBookmarks::btnEdit_OnClick);
		m_btnRemove.OnClick = Callback(this, &CJabberDlgBookmarks::btnRemove_OnClick);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_BOOKMARKS));

		m_btnAdd.Disable();
		m_btnEdit.Disable();
		m_btnRemove.Disable();

		m_lvBookmarks.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_DOUBLEBUFFER);

		HIMAGELIST hIml = m_lvBookmarks.CreateImageList(LVSIL_SMALL);
		g_plugin.addImgListIcon(hIml, IDI_GROUP);
		ImageList_AddSkinIcon(hIml, SKINICON_EVENT_URL);

		m_lvBookmarks.AddColumn(0, TranslateT("Bookmark Name"), m_proto->getWord("bookmarksWnd_cx0", 120));
		m_lvBookmarks.AddColumn(1, TranslateT("Address (JID or URL)"), m_proto->getWord("bookmarksWnd_cx1", 210));
		m_lvBookmarks.AddColumn(2, TranslateT("Nickname"), m_proto->getWord("bookmarksWnd_cx2", 90));

		m_lvBookmarks.EnableGroupView(TRUE);
		m_lvBookmarks.AddGroup(0, TranslateT("Conferences"));
		m_lvBookmarks.AddGroup(1, TranslateT("Links"));

		Utils_RestoreWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "bookmarksWnd_");
		return true;
	}

	bool OnApply() override
	{
		OpenBookmark();
		return true;
	}

	void OnDestroy() override
	{
		LVCOLUMN lvc = { 0 };
		lvc.mask = LVCF_WIDTH;
		m_lvBookmarks.GetColumn(0, &lvc);
		m_proto->setWord("bookmarksWnd_cx0", lvc.cx);
		m_lvBookmarks.GetColumn(1, &lvc);
		m_proto->setWord("bookmarksWnd_cx1", lvc.cx);
		m_lvBookmarks.GetColumn(2, &lvc);
		m_proto->setWord("bookmarksWnd_cx2", lvc.cx);

		Utils_SaveWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "bookmarksWnd_");

		m_proto->m_pDlgBookmarks = nullptr;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_BM_LIST:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

		case IDCANCEL:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

		case IDC_ADD:
		case IDC_EDIT:
		case IDC_REMOVE:
			return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
		}
		return CSuper::Resizer(urc);
	}

	void lvBookmarks_OnDoubleClick(CCtrlButton*)
	{
		OpenBookmark();
	}

	void btnAdd_OnClick(CCtrlButton*)
	{
		if (!m_proto->m_bJabberOnline) return;

		JabberAddBookmarkDlgParam param;
		param.ppro = m_proto;
		param.m_item = nullptr;
		DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_BOOKMARK_ADD), m_hwnd, JabberAddBookmarkDlgProc, (LPARAM)&param);
	}

	void btnEdit_OnClick(CCtrlButton*)
	{
		if (!m_proto->m_bJabberOnline)
			return;

		int iItem = m_lvBookmarks.GetNextItem(-1, LVNI_SELECTED);
		if (iItem < 0)
			return;

		char *address = (char*)m_lvBookmarks.GetItemData(iItem);
		if (address == nullptr)
			return;

		JABBER_LIST_ITEM *item = m_proto->ListGetItemPtr(LIST_BOOKMARK, address);
		if (item == nullptr)
			return;

		JabberAddBookmarkDlgParam param;
		param.ppro = m_proto;
		param.m_item = item;
		DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_BOOKMARK_ADD), m_hwnd, JabberAddBookmarkDlgProc, (LPARAM)&param);
	}

	void btnRemove_OnClick(CCtrlButton*)
	{
		if (!m_proto->m_bJabberOnline)
			return;

		int iItem = m_lvBookmarks.GetNextItem(-1, LVNI_SELECTED);
		if (iItem < 0)
			return;

		char *address = (char*)m_lvBookmarks.GetItemData(iItem);
		if (address == nullptr)
			return;

		JABBER_LIST_ITEM *item = m_proto->ListGetItemPtr(LIST_BOOKMARK, address);
		if (item == nullptr)
			return;

		m_btnAdd.Disable();
		m_btnEdit.Disable();
		m_btnRemove.Disable();

		m_proto->ListRemove(LIST_BOOKMARK, address);

		m_lvBookmarks.SetItemState(iItem, 0, LVIS_SELECTED); // Unselect the item

		XmlNodeIq iq(m_proto->AddIQ(&CJabberProto::OnIqResultSetBookmarks, JABBER_IQ_TYPE_SET));
		m_proto->SetBookmarkRequest(iq);
		m_proto->m_ThreadInfo->send(iq);
	}

	void OnProtoRefresh(WPARAM, LPARAM) override
	{
		m_lvBookmarks.DeleteAllItems();

		JABBER_LIST_ITEM *item = nullptr;
		LISTFOREACH(i, m_proto, LIST_BOOKMARK)
		{
			if (item = m_proto->ListGetItemPtrFromIndex(i)) {
				int itemType = mir_strcmpi(item->type, "conference") ? 1 : 0;
				m_proto->debugLogA("BOOKMARK #%d: %d %s", i, itemType, item->jid);

				int iItem = m_lvBookmarks.AddItem(item->name, itemType, (LPARAM)item->jid, itemType);
				m_lvBookmarks.SetItem(iItem, 1, Utf2T(item->jid));
				if (itemType == 0)
					m_lvBookmarks.SetItem(iItem, 2, Utf2T(item->nick));
			}
		}

		if (item) {
			m_btnEdit.Enable();
			m_btnRemove.Enable();
		}

		m_btnAdd.Enable();
	}

	void OnProtoCheckOnline(WPARAM, LPARAM) override
	{
		if (!m_proto->m_bJabberOnline) {
			m_btnAdd.Disable();
			m_btnEdit.Disable();
			m_btnRemove.Disable();
		}
		else UpdateData();
	}

	void OpenBookmark()
	{
		int iItem = m_lvBookmarks.GetNextItem(-1, LVNI_SELECTED);
		if (iItem < 0)
			return;

		char *address = (char*)m_lvBookmarks.GetItemData(iItem);
		if (address == nullptr)
			return;

		JABBER_LIST_ITEM *item = m_proto->ListGetItemPtr(LIST_BOOKMARK, address);
		if (item == nullptr)
			return;

		if (!mir_strcmpi(item->type, "conference")) {
			m_lvBookmarks.SetItemState(iItem, 0, LVIS_SELECTED); // Unselect the item

			// some hack for using bookmark to transport not under XEP-0048
			if (!strchr(item->jid, '@'))
				//the room name is not provided let consider that it is transport and send request to registration
				m_proto->RegisterAgent(nullptr, item->jid);
			else {
				if (auto *pRoom = m_proto->ListGetItemPtr(LIST_CHATROOM, item->jid))
					if (pRoom->hContact != 0) {
						Clist_ContactDoubleClicked(pRoom->hContact);
						return;
					}

				char *room = NEWSTR_ALLOCA(item->jid);
				char *server = strchr(room, '@');
				*server++ = 0;

				if (item->nick && *item->nick)
					m_proto->GroupchatJoinRoom(server, room, item->nick, item->password);
				else
					m_proto->GroupchatJoinRoom(server, room, ptrA(JabberNickFromJID(m_proto->m_szJabberJID)), item->password);
			}
		}
		else Utils_OpenUrlW(Utf2T(item->jid));
	}

	void UpdateData()
	{
		if (!m_proto->m_bJabberOnline) return;

		m_proto->m_ThreadInfo->send(
			XmlNodeIq(m_proto->AddIQ(&CJabberProto::OnIqResultDiscoBookmarks, JABBER_IQ_TYPE_GET))
			<< XQUERY(JABBER_FEAT_PRIVATE_STORAGE) << XCHILDNS("storage", "storage:bookmarks"));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Launches the Bookmarks manager window

INT_PTR __cdecl CJabberProto::OnMenuHandleBookmarks(WPARAM, LPARAM)
{
	UI_SAFE_OPEN_EX(CJabberDlgBookmarks, m_pDlgBookmarks, pDlg);
	pDlg->UpdateData();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Launches the Bookmark details window, lParam is JABBER_BOOKMARK_ITEM*

int CJabberProto::AddEditBookmark(JABBER_LIST_ITEM *item)
{
	if (m_bJabberOnline) {
		JabberAddBookmarkDlgParam param;
		param.ppro = this;
		param.m_item = item;//(JABBER_LIST_ITEM*)lParam;
		DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_BOOKMARK_ADD), nullptr, JabberAddBookmarkDlgProc, (LPARAM)&param);
	}
	return 0;
}
