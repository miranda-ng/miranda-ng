/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (C) 2012-26 Miranda NG team

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
#include "jabber_list.h"
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_privacy.h"
#include "jabber_notes.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CNoteItem class members

CNoteItem::CNoteItem()
{
}

CNoteItem::CNoteItem(const TiXmlElement *hXml, const char *szFrom)
{
	SetData(
		XmlGetChildText(hXml, "title"),
		szFrom ? szFrom : XmlGetAttr(hXml, "from"),
		Utf2T(XmlGetChildText(hXml, "text")),
		XmlGetAttr(hXml, "tags"));
}

CNoteItem::~CNoteItem()
{
	mir_free(m_szTitle);
	mir_free(m_szFrom);
	mir_free(m_szText);
	mir_free(m_szTags);
	mir_free(m_szTagsStr);
}

void CNoteItem::SetData(const char *title, const char *from, const wchar_t*text, const char *tags)
{
	mir_free(m_szTitle);
	mir_free(m_szFrom);
	mir_free(m_szText);
	mir_free(m_szTags);
	mir_free(m_szTagsStr);

	m_szTitle = rtrim(mir_strdup(title));
	m_szText = JabberStrFixLines(text);
	m_szFrom = rtrim(mir_strdup(from));

	auto *szTags = tags;
	auto *p = m_szTags = (char *)mir_alloc(mir_strlen(szTags) + 2);
	auto *q = m_szTagsStr = (char *)mir_alloc(mir_strlen(szTags) + 1);
	for (; szTags && *szTags; ++szTags) {
		if (isspace(*szTags))
			continue;

		if (*szTags == ',') {
			*q++ = ',';
			*p++ = 0;
			continue;
		}

		*q++ = *p++ = *szTags;
	}

	q[0] = p[0] = p[1] = 0;
}

bool CNoteItem::HasTag(const char *szTag)
{
	if (!szTag || !*szTag)
		return true;

	for (auto *p = m_szTags; p && *p; p = p + mir_strlen(p) + 1)
		if (!mir_strcmp(p, szTag))
			return true;

	return false;
}

int CNoteItem::cmp(const CNoteItem *p1, const CNoteItem *p2)
{
	int ret = 0;
	if (ret = mir_strcmp(p1->m_szTitle, p2->m_szTitle)) return ret;
	if (ret = mir_wstrcmp(p1->m_szText, p2->m_szText)) return ret;
	if (ret = mir_strcmp(p1->m_szTagsStr, p2->m_szTagsStr)) return ret;
	if (p1 < p2) return -1;
	if (p1 > p2) return 1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CNoteList class members

void CNoteList::LoadXml(const TiXmlElement *hXml)
{
	destroy();
	m_bIsModified = false;

	for (auto *it : TiXmlEnum(hXml)) {
		CNoteItem *pNote = new CNoteItem(it);
		if (pNote->IsNotEmpty())
			insert(pNote);
		else
			delete pNote;
	}
}

void CNoteList::SaveXml(TiXmlElement *hXmlParent)
{
	m_bIsModified = false;

	for (auto &it : *this) {
		TiXmlElement *hXmlItem = hXmlParent << XCHILD("note");
		hXmlItem << XATTR("from", it->GetFrom()) << XATTR("tags", it->GetTagsStr());
		hXmlItem << XCHILD("title", it->GetTitle());
		hXmlItem << XCHILD("text", T2Utf(it->GetText()));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Single note editor

class CJabberDlgNoteItem : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CNoteItem *m_pNote;

	CCtrlEdit m_txtTitle;
	CCtrlEdit m_txtText;
	CCtrlEdit m_txtTags;

public:
	CJabberDlgNoteItem(CJabberDlgBase *parent, CNoteItem *pNote) :
		CSuper(parent->GetProto(), IDD_NOTE_EDIT),
		m_pNote(pNote),
		m_txtTitle(this, IDC_TXT_TITLE),
		m_txtText(this, IDC_TXT_TEXT),
		m_txtTags(this, IDC_TXT_TAGS)
	{
		SetParent(parent->GetHwnd());
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();
		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_NOTES));

		m_txtTitle.SetText(Utf2T(m_pNote->GetTitle()));
		m_txtText.SetText(m_pNote->GetText());
		m_txtTags.SetText(Utf2T(m_pNote->GetTagsStr()));
		return true;
	}

	bool OnApply() override
	{
		T2Utf szTitle(ptrW(m_txtTitle.GetText()));
		T2Utf szTags(ptrW(m_txtTags.GetText()));
		m_pNote->SetData(szTitle, m_pNote->GetFrom(), ptrW(m_txtText.GetText()), szTags);

		m_autoClose = false;
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_TXT_TITLE:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
		case IDC_TXT_TEXT:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		case IDC_ST_TAGS:
		case IDC_TXT_TAGS:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

		case IDOK:
		case IDCANCEL:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
		}

		return CSuper::Resizer(urc);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Notebook window

class CCtrlNotebookList : public CCtrlListBox
{
	typedef CCtrlListBox CSuper;
	bool m_adding;
	HFONT m_hfntNormal, m_hfntSmall, m_hfntBold;

public:
	CCtrlNotebookList(CDlgBase* dlg, int ctrlId) :
		CCtrlListBox(dlg, ctrlId),
		m_adding(false)
	{
		m_hfntNormal = m_hfntSmall = m_hfntBold = nullptr;
	}

	void SetFonts(HFONT hfntNormal, HFONT hfntSmall, HFONT hfntBold)
	{
		m_hfntNormal = hfntNormal;
		m_hfntSmall = hfntSmall;
		m_hfntBold = hfntBold;
	}

	int AddString(wchar_t *text, LPARAM data = 0)
	{
		m_adding = true;
		int idx = CCtrlListBox::AddString(text, data);
		m_adding = false;
		if (idx == LB_ERR) return idx;

		MEASUREITEMSTRUCT mis = { 0 };
		mis.CtlType = ODT_LISTBOX;
		mis.CtlID = m_idCtrl;
		mis.itemID = idx;
		mis.itemData = data;
		OnMeasureItem(&mis);
		if (mis.itemHeight)
			SendMessage(m_hwnd, LB_SETITEMHEIGHT, idx, mis.itemHeight);
		return idx;
	}

	void OnInit()
	{
		CSuper::OnInit();
		Subclass();
	}

	LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_SIZE) {
			SendMessage(m_hwnd, WM_SETREDRAW, FALSE, 0);
			int cnt = GetCount();
			for (int idx = 0; idx < cnt; ++idx) {
				MEASUREITEMSTRUCT mis = { 0 };
				mis.CtlType = ODT_LISTBOX;
				mis.CtlID = m_idCtrl;
				mis.itemID = idx;
				mis.itemData = GetItemData(idx);
				OnMeasureItem(&mis);
				if (mis.itemHeight)
					SendMessage(m_hwnd, LB_SETITEMHEIGHT, idx, mis.itemHeight);
			}
			SendMessage(m_hwnd, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE);
		}

		return CSuper::CustomWndProc(msg, wParam, lParam);
	}

	BOOL OnDrawItem(DRAWITEMSTRUCT *lps)
	{
		if (m_adding) return FALSE;
		if (lps->itemID == -1) return TRUE;
		if (!lps->itemData) return TRUE;

		HDC hdc = lps->hDC;
		CNoteItem *pNote = (CNoteItem *)lps->itemData;

		SetBkMode(hdc, TRANSPARENT);
		if (lps->itemState & ODS_SELECTED) {
			FillRect(hdc, &lps->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
			SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else {
			FillRect(hdc, &lps->rcItem, GetSysColorBrush(COLOR_WINDOW));
			SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
		}

		if (lps->itemID) {
			RECT rcTmp = lps->rcItem; rcTmp.bottom = rcTmp.top + 1;
			FillRect(hdc, &rcTmp, GetSysColorBrush(COLOR_BTNSHADOW));
		}

		RECT rc = lps->rcItem;
		rc.left += 5;
		rc.right -= 5;
		rc.top += 2;

		SelectObject(hdc, m_hfntBold);
		rc.top += DrawText(hdc, Utf2T(pNote->GetTitle()), -1, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
		SelectObject(hdc, m_hfntNormal);
		if (pNote->GetFrom()) {
			wchar_t buf[256];
			mir_snwprintf(buf, TranslateT("From: %s"), pNote->GetFrom());
			rc.top += DrawText(hdc, buf, -1, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
		}
		rc.top += DrawText(hdc, pNote->GetText(), -1, &rc, DT_NOPREFIX | DT_WORDBREAK | DT_EXPANDTABS | DT_END_ELLIPSIS);
		SelectObject(hdc, m_hfntSmall);
		rc.top += DrawText(hdc, Utf2T(pNote->GetTagsStr()), -1, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
		rc.top += 5;

		int h = min(255, max(0, rc.bottom - rc.top));
		if (SendMessage(m_hwnd, LB_GETITEMHEIGHT, lps->itemID, 0) != h)
			SendMessage(m_hwnd, LB_SETITEMHEIGHT, lps->itemID, h);

		return TRUE;
	}

	BOOL OnMeasureItem(MEASUREITEMSTRUCT *lps)
	{
		if (m_adding) return FALSE;
		if (lps->itemID == -1) return TRUE;
		if (!lps->itemData) return TRUE;

		HDC hdc = GetDC(m_hwnd);
		CNoteItem *pNote = (CNoteItem *)lps->itemData;

		RECT rcTmp, rc;
		GetClientRect(m_hwnd, &rc);
		rc.bottom = 0;

		SelectObject(hdc, m_hfntBold);
		rcTmp = rc;
		DrawText(hdc, Utf2T(pNote->GetTitle()), -1, &rcTmp, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT);
		lps->itemHeight += rcTmp.bottom;
		SelectObject(hdc, m_hfntNormal);
		if (pNote->GetFrom()) {
			wchar_t buf[256];
			mir_snwprintf(buf, TranslateT("From: %s"), pNote->GetFrom());
			rcTmp = rc;
			DrawText(hdc, buf, -1, &rcTmp, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT);
			lps->itemHeight += rcTmp.bottom;
		}
		rcTmp = rc;
		DrawText(hdc, pNote->GetText(), -1, &rcTmp, DT_NOPREFIX | DT_WORDBREAK | DT_EXPANDTABS | DT_END_ELLIPSIS | DT_CALCRECT);
		lps->itemHeight += rcTmp.bottom;
		SelectObject(hdc, m_hfntSmall);
		rcTmp = rc;
		DrawText(hdc, Utf2T(pNote->GetTagsStr()), -1, &rcTmp, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT);
		lps->itemHeight += rcTmp.bottom;
		lps->itemHeight += 5;

		ReleaseDC(m_hwnd, hdc);

		lps->itemWidth = rc.right;
		lps->itemHeight = min(255, lps->itemHeight); // listbox can't make items taller then 255px
		return TRUE;
	}
};

class CJabberDlgNotes : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlMButton		m_btnAdd;
	CCtrlMButton		m_btnEdit;
	CCtrlMButton		m_btnRemove;
	CCtrlNotebookList	m_lstNotes;
	CCtrlTreeView		m_tvFilter;
	CCtrlButton			m_btnSave;

	HFONT m_hfntNormal, m_hfntSmall, m_hfntBold;
	CNoteList m_notes;

	void EnableControls()
	{
		m_btnSave.Enable(m_proto->m_bJabberOnline && m_notes.IsModified());
		m_btnEdit.Enable(m_lstNotes.GetCurSel() != LB_ERR);
		m_btnRemove.Enable(m_lstNotes.GetCurSel() != LB_ERR);
	}

	void InsertTag(HTREEITEM htiRoot, const char *tag, bool bSelect)
	{
		Utf2T wszTag(tag);

		TVINSERTSTRUCT tvi = {};
		tvi.hParent = htiRoot;
		tvi.hInsertAfter = TVI_LAST;
		tvi.itemex.mask = TVIF_TEXT | TVIF_PARAM;
		tvi.itemex.pszText = wszTag;
		tvi.itemex.lParam = (LPARAM)mir_strdup(tag);
		HTREEITEM hti = m_tvFilter.InsertItem(&tvi);
		if (bSelect) m_tvFilter.SelectItem(hti);
	}

	void PopulateTags(HTREEITEM htiRoot, const char *szActiveTag)
	{
		LIST<char> tagSet(5, strcmp);
		for (auto &it : m_notes) {
			char *tags = it->GetTags();
			for (auto *tag = tags; tag && *tag; tag = tag + mir_strlen(tag) + 1)
				if (!tagSet.find(tag))
					tagSet.insert(tag);
		}

		bool selected = false;
		for (auto &it : tagSet) {
			bool select = !mir_strcmp(szActiveTag, it);
			selected |= select;
			InsertTag(htiRoot, it, select);
		}

		if (!selected)
			m_tvFilter.SelectItem(htiRoot);
	}

	void RebuildTree()
	{
		TVITEMEX tvi = { 0 };
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = m_tvFilter.GetSelection();
		m_tvFilter.GetItem(&tvi);
		char *szActiveTag = mir_strdup((char*)tvi.lParam);

		m_tvFilter.DeleteAllItems();

		TVINSERTSTRUCT tvis = {};
		tvis.hInsertAfter = TVI_LAST;
		tvis.itemex.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
		tvis.itemex.stateMask = tvis.itemex.state = TVIS_BOLD | TVIS_EXPANDED;
		tvis.itemex.pszText = TranslateT("All tags");

		PopulateTags(m_tvFilter.InsertItem(&tvis), szActiveTag);
		mir_free(szActiveTag);
	}

	void InsertItem(CNoteItem &item)
	{
		m_lstNotes.AddString((wchar_t *)item.GetTitle(), (LPARAM)&item);
		EnableControls();
	}

	void ListItems(const char *tag)
	{
		m_lstNotes.ResetContent();
		for (auto &it : m_notes)
			if (it->HasTag(tag))
				InsertItem(*it);
		EnableControls();
	}

public:
	CJabberDlgNotes(CJabberProto *proto) :
		CSuper(proto, IDD_NOTEBOOK),
		m_btnAdd(this, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add")),
		m_btnEdit(this, IDC_EDIT, SKINICON_OTHER_EDIT, LPGEN("Edit")),
		m_btnRemove(this, IDC_REMOVE, SKINICON_OTHER_DELETE, LPGEN("Remove")),
		m_lstNotes(this, IDC_LST_NOTES),
		m_tvFilter(this, IDC_TV_FILTER),
		m_btnSave(this, IDC_APPLY)
	{
		m_btnAdd.OnClick = Callback(this, &CJabberDlgNotes::btnAdd_OnClick);
		m_btnEdit.OnClick = Callback(this, &CJabberDlgNotes::btnEdit_OnClick);
		m_btnRemove.OnClick = Callback(this, &CJabberDlgNotes::btnRemove_OnClick);
		m_lstNotes.OnDblClick = Callback(this, &CJabberDlgNotes::btnEdit_OnClick);
		m_lstNotes.OnSelChange = Callback(this, &CJabberDlgNotes::lstNotes_OnSelChange);
		m_btnSave.OnClick = Callback(this, &CJabberDlgNotes::btnSave_OnClick);

		m_tvFilter.OnSelChanged = Callback(this, &CJabberDlgNotes::tvFilter_OnSelChanged);
		m_tvFilter.OnDeleteItem = Callback(this, &CJabberDlgNotes::tvFilter_OnDeleteItem);
	}
	
	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();
		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_NOTES));

		// Server-side notes
		m_proto->m_ThreadInfo->send(
			XmlNodeIq(m_proto->AddIQ(&CJabberProto::OnIqResultNotes, JABBER_IQ_TYPE_GET, 0, this))
			<< XQUERY(JABBER_FEAT_PRIVATE_STORAGE)
			<< XCHILDNS("storage", JABBER_FEAT_MIRANDA_NOTES));

		LOGFONT lf, lfTmp;
		m_hfntNormal = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		GetObject(m_hfntNormal, sizeof(lf), &lf);
		lfTmp = lf; lfTmp.lfWeight = FW_BOLD;
		m_hfntBold = CreateFontIndirect(&lfTmp);
		lfTmp = lf; lfTmp.lfHeight *= 0.8;
		m_hfntSmall = CreateFontIndirect(&lfTmp);
		m_lstNotes.SetFonts(m_hfntNormal, m_hfntSmall, m_hfntBold);

		Utils_RestoreWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "notesWnd_");
		return true;
	}

	bool OnClose() override
	{
		if (m_notes.IsModified())
			if (IDYES != MessageBox(m_hwnd, TranslateT("Notes are not saved, close this window without uploading data to server?"), TranslateT("Are you sure?"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2))
				return false;

		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "notesWnd_");

		DeleteObject(m_hfntSmall);
		DeleteObject(m_hfntBold);

		m_tvFilter.DeleteAllItems();
		m_proto->m_pDlgNotes = nullptr;
		CSuper::OnDestroy();
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_TV_FILTER:
			return RD_ANCHORX_LEFT | RD_ANCHORY_HEIGHT;
		case IDC_LST_NOTES:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		case IDC_APPLY:
		case IDCANCEL:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
		case IDC_ADD:
		case IDC_EDIT:
		case IDC_REMOVE:
			return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
		}
		return CSuper::Resizer(urc);
	}

	void OnProtoCheckOnline(WPARAM, LPARAM) override
	{
		EnableControls();
	}

	void btnAdd_OnClick(CCtrlButton*)
	{
		CNoteItem *pNote = new CNoteItem();
		CJabberDlgNoteItem dlg(this, pNote);
		dlg.DoModal();

		if (pNote->IsNotEmpty()) {
			m_notes.insert(pNote);
			m_notes.Modify();
			UpdateData();
		}
		else {
			delete pNote;
			return;
		}
		EnableControls();
	}

	void btnEdit_OnClick(CCtrlButton*)
	{
		int idx = m_lstNotes.GetCurSel();
		if (idx != LB_ERR) {
			if (CNoteItem *pItem = (CNoteItem *)m_lstNotes.GetItemData(idx)) {
				CJabberDlgNoteItem dlg(this, pItem);
				if (dlg.DoModal()) {
					m_notes.Modify();
					RebuildTree();
				}
			}
		}
		EnableControls();
	}

	void btnRemove_OnClick(CCtrlButton*)
	{
		int idx = m_lstNotes.GetCurSel();
		if (idx != LB_ERR) {
			if (CNoteItem *pItem = (CNoteItem *)m_lstNotes.GetItemData(idx)) {
				m_lstNotes.DeleteString(idx);
				m_notes.remove(pItem);
			}
			RebuildTree();
		}
		EnableControls();
	}

	void lstNotes_OnSelChange(CCtrlListBox *)
	{
		EnableControls();
	}

	void tvFilter_OnDeleteItem(CCtrlTreeView::TEventInfo *e)
	{
		char *szText = (char*)e->nmtv->itemNew.lParam;
		mir_free(szText);
		EnableControls();
	}

	void tvFilter_OnSelChanged(CCtrlTreeView::TEventInfo *e)
	{
		char *szText = (char*)e->nmtv->itemNew.lParam;
		ListItems(szText);
		EnableControls();
	}

	void btnSave_OnClick(CCtrlButton*)
	{
		XmlNodeIq iq("set");
		TiXmlElement *query = iq << XQUERY(JABBER_FEAT_PRIVATE_STORAGE);
		TiXmlElement *storage = query << XCHILDNS("storage", JABBER_FEAT_MIRANDA_NOTES);
		m_notes.SaveXml(storage);
		m_proto->m_ThreadInfo->send(iq);
		EnableControls();
	}

	void UpdateData(const TiXmlElement *storage = 0)
	{
		if (storage)
			m_notes.LoadXml(storage);

		RebuildTree();
		EnableControls();
	}
};

void CJabberProto::OnIqResultNotes(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	auto *pDlg = (CJabberDlgNotes *)pInfo->GetUserData();

	if (iqNode && pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT)
		if (auto *query = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_PRIVATE_STORAGE))
			if (auto *storage = XmlGetChildByTag(query, "storage", "xmlns", JABBER_FEAT_MIRANDA_NOTES))
				pDlg->UpdateData(storage);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Menu handling

INT_PTR __cdecl CJabberProto::OnMenuHandleNotes(WPARAM, LPARAM)
{
	UI_SAFE_OPEN(CJabberDlgNotes, m_pDlgNotes);
	return 0;
}
