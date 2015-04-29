/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (ñ) 2012-15 Miranda NG project

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

static TCHAR* StrTrimCopy(TCHAR *str)
{
	if (!str) return 0;
	while (*str && _istspace(*str)) ++str;
	if (!*str) return mir_tstrdup(str);

	TCHAR *res = mir_tstrdup(str);
	for (TCHAR *p = res + mir_tstrlen(res) - 1; p >= res; --p) {
		if (_istspace(*p))
			*p = 0;
		else
			break;
	}

	return res;
}

CNoteItem::CNoteItem()
{
	m_szTitle = m_szFrom = m_szText = m_szTags = m_szTagsStr = NULL;
}

CNoteItem::CNoteItem(HXML hXml, TCHAR *szFrom)
{
	m_szTitle = m_szFrom = m_szText = m_szTags = m_szTagsStr = NULL;
	SetData(
		XPathT(hXml, "title"),
		szFrom ? szFrom : XPathT(hXml, "@from"),
		XPathT(hXml, "text"),
		XPathT(hXml, "@tags"));
}

CNoteItem::~CNoteItem()
{
	mir_free(m_szTitle);
	mir_free(m_szFrom);
	mir_free(m_szText);
	mir_free(m_szTags);
	mir_free(m_szTagsStr);
}

void CNoteItem::SetData(TCHAR *title, TCHAR *from, TCHAR *text, TCHAR *tags)
{
	mir_free(m_szTitle);
	mir_free(m_szFrom);
	mir_free(m_szText);
	mir_free(m_szTags);
	mir_free(m_szTagsStr);

	m_szTitle = StrTrimCopy(title);
	m_szText = JabberStrFixLines(text);
	m_szFrom = StrTrimCopy(from);

	const TCHAR *szTags = tags;
	TCHAR *p = m_szTags = (TCHAR *)mir_alloc((mir_tstrlen(szTags) + 2 /*for double zero*/) * sizeof(TCHAR));
	TCHAR *q = m_szTagsStr = (TCHAR *)mir_alloc((mir_tstrlen(szTags) + 1) * sizeof(TCHAR));
	for (; szTags && *szTags; ++szTags) {
		if (_istspace(*szTags))
			continue;

		if (*szTags == _T(',')) {
			*q++ = _T(',');
			*p++ = 0;
			continue;
		}

		*q++ = *p++ = *szTags;
	}

	q[0] = p[0] = p[1] = 0;
}

bool CNoteItem::HasTag(const TCHAR *szTag)
{
	if (!szTag || !*szTag)
		return true;

	for (TCHAR *p = m_szTags; p && *p; p = p + mir_tstrlen(p) + 1)
		if (!mir_tstrcmp(p, szTag))
			return true;

	return false;
}

int CNoteItem::cmp(const CNoteItem *p1, const CNoteItem *p2)
{
	int ret = 0;
	if (ret = mir_tstrcmp(p1->m_szTitle, p2->m_szTitle)) return ret;
	if (ret = mir_tstrcmp(p1->m_szText, p2->m_szText)) return ret;
	if (ret = mir_tstrcmp(p1->m_szTagsStr, p2->m_szTagsStr)) return ret;
	if (p1 < p2) return -1;
	if (p1 > p2) return 1;
	return 0;
}

void CNoteList::AddNote(HXML hXml, TCHAR *szFrom)
{
	m_bIsModified = true;
	insert(new CNoteItem(hXml, szFrom));
}

void CNoteList::LoadXml(HXML hXml)
{
	destroy();
	m_bIsModified = false;

	int count = xmlGetChildCount(hXml);
	for (int i=0; i < count; i++)
	{
		CNoteItem *pNote = new CNoteItem(xi.getChild(hXml, i));
		if (pNote->IsNotEmpty())
			insert(pNote);
		else
			delete pNote;
	}
}

void CNoteList::SaveXml(HXML hXmlParent)
{
	m_bIsModified = false;
	CNoteList &me = *this;

	for (int i=0; i < getCount(); i++)
	{
		HXML hXmlItem = hXmlParent << XCHILD(_T("note"));
		hXmlItem << XATTR(_T("from"), me[i].GetFrom()) << XATTR(_T("tags"), me[i].GetTagsStr());
		hXmlItem << XCHILD(_T("title"), me[i].GetTitle());
		hXmlItem << XCHILD(_T("text"), me[i].GetText());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Single note editor

class CJabberDlgNoteItem : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;
	typedef void (CJabberProto::*TFnProcessNote)(CNoteItem *, bool ok);

public:
	CJabberDlgNoteItem(CJabberDlgBase *parent, CNoteItem *pNote);
	CJabberDlgNoteItem(CJabberProto *proto, CNoteItem *pNote, TFnProcessNote fnProcess);

protected:
	void OnInitDialog();
	int Resizer(UTILRESIZECONTROL *urc);

private:
	CNoteItem *m_pNote;
	TFnProcessNote m_fnProcess;

	CCtrlEdit m_txtTitle;
	CCtrlEdit m_txtText;
	CCtrlEdit m_txtTags;
	CCtrlButton m_btnOk;

	void btnOk_OnClick(CCtrlButton *)
	{
		TCHAR *szTitle = m_txtTitle.GetText();
		TCHAR *szText = m_txtText.GetText();
		TCHAR *szTags = m_txtTags.GetText();
		TCHAR *szFrom = mir_tstrdup(m_pNote->GetFrom());
		m_pNote->SetData(szTitle, szFrom, szText, szTags);
		mir_free(szTitle);
		mir_free(szText);
		mir_free(szTags);
		mir_free(szFrom);

		m_autoClose = false;
		if (m_fnProcess) (m_proto->*m_fnProcess)(m_pNote, true);
		EndDialog(m_hwnd, TRUE);
	}

	void OnClose()
	{
		if (m_fnProcess) (m_proto->*m_fnProcess)(m_pNote, false);
		CSuper::OnClose();
	}
};

CJabberDlgNoteItem::CJabberDlgNoteItem(CJabberDlgBase *parent, CNoteItem *pNote):
	CSuper(parent->GetProto(), IDD_NOTE_EDIT),
	m_pNote(pNote),
	m_fnProcess(NULL),
	m_txtTitle(this, IDC_TXT_TITLE),
	m_txtText(this, IDC_TXT_TEXT),
	m_txtTags(this, IDC_TXT_TAGS),
	m_btnOk(this, IDOK)
{
	SetParent(parent->GetHwnd());
	m_btnOk.OnClick = Callback(this, &CJabberDlgNoteItem::btnOk_OnClick);
}

CJabberDlgNoteItem::CJabberDlgNoteItem(CJabberProto *proto, CNoteItem *pNote, TFnProcessNote fnProcess):
	CSuper(proto, IDD_NOTE_EDIT),
	m_pNote(pNote),
	m_fnProcess(fnProcess),
	m_txtTitle(this, IDC_TXT_TITLE),
	m_txtText(this, IDC_TXT_TEXT),
	m_txtTags(this, IDC_TXT_TAGS),
	m_btnOk(this, IDOK)
{
	m_btnOk.OnClick = Callback(this, &CJabberDlgNoteItem::btnOk_OnClick);
}

void CJabberDlgNoteItem::OnInitDialog()
{
	CSuper::OnInitDialog();
	WindowSetIcon(m_hwnd, m_proto, "notes");

	if (m_fnProcess) {
		CMString buf;
		if (m_fnProcess == &CJabberProto::ProcessIncomingNote)
			buf.Format(TranslateT("Incoming note from %s"), m_pNote->GetFrom());
		else
			buf.Format(TranslateT("Send note to %s"), m_pNote->GetFrom());

		SetWindowText(m_hwnd, buf);
	}

	m_txtTitle.SetText(m_pNote->GetTitle());
	m_txtText.SetText(m_pNote->GetText());
	m_txtTags.SetText(m_pNote->GetTagsStr());
}

int CJabberDlgNoteItem::Resizer(UTILRESIZECONTROL *urc)
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

/////////////////////////////////////////////////////////////////////////////////////////
// Notebook window

class CCtrlNotebookList: public CCtrlListBox
{
	typedef CCtrlListBox CSuper;
	bool m_adding;
	HFONT m_hfntNormal, m_hfntSmall, m_hfntBold;

public:
	CCtrlNotebookList(CDlgBase* dlg, int ctrlId)
		: CCtrlListBox(dlg, ctrlId),
		m_adding(false)
	{
		m_hfntNormal = m_hfntSmall = m_hfntBold = NULL;
	}
	
	void SetFonts(HFONT hfntNormal, HFONT hfntSmall, HFONT hfntBold)
	{
		m_hfntNormal = hfntNormal;
		m_hfntSmall = hfntSmall;
		m_hfntBold = hfntBold;
	}

	int AddString(TCHAR *text, LPARAM data = 0)
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
		if (mis.itemHeight) SendMessage(m_hwnd, LB_SETITEMHEIGHT, idx, mis.itemHeight);
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
				if (mis.itemHeight) SendMessage(m_hwnd, LB_SETITEMHEIGHT, idx, mis.itemHeight);
			}
			SendMessage(m_hwnd, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
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
		rc.top += DrawText(hdc, pNote->GetTitle(), -1, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
		SelectObject(hdc, m_hfntNormal);
		if (pNote->GetFrom()) {
			TCHAR buf[256];
			mir_sntprintf(buf, SIZEOF(buf), TranslateT("From: %s"), pNote->GetFrom());
			rc.top += DrawText(hdc, buf, -1, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
		}
		rc.top += DrawText(hdc, pNote->GetText(), -1, &rc, DT_NOPREFIX | DT_WORDBREAK | DT_EXPANDTABS | DT_END_ELLIPSIS);
		SelectObject(hdc, m_hfntSmall);
		rc.top += DrawText(hdc, pNote->GetTagsStr(), -1, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
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
		DrawText(hdc, pNote->GetTitle(), -1, &rcTmp, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT);
		lps->itemHeight += rcTmp.bottom;
		SelectObject(hdc, m_hfntNormal);
		if (pNote->GetFrom()) {
			TCHAR buf[256];
			mir_sntprintf(buf, SIZEOF(buf), TranslateT("From: %s"), pNote->GetFrom());
			rcTmp = rc;
			DrawText(hdc, buf, -1, &rcTmp, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT);
			lps->itemHeight += rcTmp.bottom;
		}
		rcTmp = rc;
		DrawText(hdc, pNote->GetText(), -1, &rcTmp, DT_NOPREFIX | DT_WORDBREAK | DT_EXPANDTABS | DT_END_ELLIPSIS | DT_CALCRECT);
		lps->itemHeight += rcTmp.bottom;
		SelectObject(hdc, m_hfntSmall);
		rcTmp = rc;
		DrawText(hdc, pNote->GetTagsStr(), -1, &rcTmp, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT);
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

public:
	CJabberDlgNotes(CJabberProto *proto);
	void UpdateData();

protected:
	void OnInitDialog();
	void OnClose();
	void OnDestroy();
	int Resizer(UTILRESIZECONTROL *urc);

	void OnProtoCheckOnline(WPARAM wParam, LPARAM lParam);
	void OnProtoRefresh(WPARAM wParam, LPARAM lParam);

private:
	CCtrlMButton		m_btnAdd;
	CCtrlMButton		m_btnEdit;
	CCtrlMButton		m_btnRemove;
	CCtrlNotebookList	m_lstNotes;
	CCtrlTreeView		m_tvFilter;
	CCtrlButton			m_btnSave;

	HFONT m_hfntNormal, m_hfntSmall, m_hfntBold;

	void EnableControls()
	{
		m_btnSave.Enable(m_proto->m_bJabberOnline && m_proto->m_notes.IsModified());
		m_btnEdit.Enable(m_lstNotes.GetCurSel() != LB_ERR);
		m_btnRemove.Enable(m_lstNotes.GetCurSel() != LB_ERR);
	}

	void InsertTag(HTREEITEM htiRoot, const TCHAR *tag, bool bSelect)
	{
		TVINSERTSTRUCT tvi = { 0 };
		tvi.hParent = htiRoot;
		tvi.hInsertAfter = TVI_LAST;
		tvi.itemex.mask = TVIF_TEXT | TVIF_PARAM;
		tvi.itemex.pszText = (TCHAR *)tag;
		tvi.itemex.lParam = (LPARAM)mir_tstrdup(tag);
		HTREEITEM hti = m_tvFilter.InsertItem(&tvi);
		if (bSelect) m_tvFilter.SelectItem(hti);
	}

	void PopulateTags(HTREEITEM htiRoot, TCHAR *szActiveTag)
	{
		LIST<TCHAR> tagSet(5, _tcscmp);
		for (int i = 0; i < m_proto->m_notes.getCount(); i++) {
			TCHAR *tags = m_proto->m_notes[i].GetTags();
			for (TCHAR *tag = tags; tag && *tag; tag = tag + mir_tstrlen(tag) + 1)
			if (!tagSet.find(tag))
				tagSet.insert(tag);
		}

		bool selected = false;
		for (int j = 0; j < tagSet.getCount(); ++j) {
			bool select = !mir_tstrcmp(szActiveTag, tagSet[j]);
			selected |= select;
			InsertTag(htiRoot, tagSet[j], select);
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
		TCHAR *szActiveTag = mir_tstrdup((TCHAR *)tvi.lParam);

		m_tvFilter.DeleteAllItems();

		TVINSERTSTRUCT tvis = { 0 };
		tvis.hInsertAfter = TVI_LAST;
		tvis.itemex.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
		tvis.itemex.stateMask = tvis.itemex.state = TVIS_BOLD | TVIS_EXPANDED;
		tvis.itemex.pszText = TranslateT("All tags");

		PopulateTags(m_tvFilter.InsertItem(&tvis), szActiveTag);
		mir_free(szActiveTag);
	}

	void InsertItem(CNoteItem &item)
	{
		m_lstNotes.AddString((TCHAR *)item.GetTitle(), (LPARAM)&item);
		EnableControls();
	}

	void ListItems(const TCHAR *tag)
	{
		m_lstNotes.ResetContent();
		for (int i=0; i < m_proto->m_notes.getCount(); i++)
			if (m_proto->m_notes[i].HasTag(tag))
				InsertItem(m_proto->m_notes[i]);
		EnableControls();
	}

	void btnAdd_OnClick(CCtrlFilterListView *)
	{
		CNoteItem *pNote = new CNoteItem();
		CJabberDlgNoteItem dlg(this, pNote);
		dlg.DoModal();

		if (pNote->IsNotEmpty()) {
			m_proto->m_notes.insert(pNote);
			m_proto->m_notes.Modify();
			UpdateData();
		}
		else {
			delete pNote;
			return;
		}
		EnableControls();
	}

	void btnEdit_OnClick(CCtrlFilterListView *)
	{
		int idx = m_lstNotes.GetCurSel();
		if (idx != LB_ERR) {
			if (CNoteItem *pItem = (CNoteItem *)m_lstNotes.GetItemData(idx)) {
				CJabberDlgNoteItem dlg(this, pItem);
				if (dlg.DoModal()) {
					m_proto->m_notes.Modify();
					RebuildTree();
				}
			}
		}
		EnableControls();
	}

	void btnRemove_OnClick(CCtrlFilterListView *)
	{
		int idx = m_lstNotes.GetCurSel();
		if (idx != LB_ERR) {
			if (CNoteItem *pItem = (CNoteItem *)m_lstNotes.GetItemData(idx)) {
				m_lstNotes.DeleteString(idx);
				m_proto->m_notes.remove(pItem);
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
		TCHAR *szText = (TCHAR *)e->nmtv->itemOld.lParam;
		mir_free(szText);
		EnableControls();
	}

	void tvFilter_OnSelChanged(CCtrlTreeView::TEventInfo *e)
	{
		TCHAR *szText = (TCHAR *)e->nmtv->itemNew.lParam;
		ListItems(szText);
		EnableControls();
	}

	void btnSave_OnClick(CCtrlButton *)
	{
		XmlNodeIq iq(_T("set"));
		HXML query = iq << XQUERY(JABBER_FEAT_PRIVATE_STORAGE);
		HXML storage = query << XCHILDNS(_T("storage"), JABBER_FEAT_MIRANDA_NOTES);
		m_proto->m_notes.SaveXml(storage);
		m_proto->m_ThreadInfo->send(iq);
		EnableControls();
	}
};

CJabberDlgNotes::CJabberDlgNotes(CJabberProto *proto) :
	CSuper(proto, IDD_NOTEBOOK, NULL),
	m_btnAdd(this, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add")),
	m_btnEdit(this, IDC_EDIT, SKINICON_OTHER_RENAME, LPGEN("Edit")),
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

void CJabberDlgNotes::UpdateData()
{
	RebuildTree();
	EnableControls();
}

void CJabberDlgNotes::OnInitDialog()
{
	CSuper::OnInitDialog();
	WindowSetIcon(m_hwnd, m_proto, "notes");

	LOGFONT lf, lfTmp;
	m_hfntNormal = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	GetObject(m_hfntNormal, sizeof(lf), &lf);
	lfTmp = lf; lfTmp.lfWeight = FW_BOLD;
	m_hfntBold = CreateFontIndirect(&lfTmp);
	lfTmp = lf; lfTmp.lfHeight *= 0.8;
	m_hfntSmall = CreateFontIndirect(&lfTmp);
	m_lstNotes.SetFonts(m_hfntNormal, m_hfntSmall, m_hfntBold);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "notesWnd_");
}

void CJabberDlgNotes::OnClose()
{
	if (m_proto->m_notes.IsModified()) {
		if (IDYES != MessageBox(m_hwnd, TranslateT("Notes are not saved, close this window without uploading data to server?"), TranslateT("Are you sure?"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2)) {
			m_lresult = TRUE;
			return;
		}
	}

	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "notesWnd_");
	DeleteObject(m_hfntSmall);
	DeleteObject(m_hfntBold);
	CSuper::OnClose();
}

void CJabberDlgNotes::OnDestroy()
{
	m_tvFilter.DeleteAllItems();
	m_proto->m_pDlgNotes = NULL;
	CSuper::OnDestroy();
}

void CJabberDlgNotes::OnProtoCheckOnline(WPARAM, LPARAM)
{
	EnableControls();
}

void CJabberDlgNotes::OnProtoRefresh(WPARAM, LPARAM)
{
}

int CJabberDlgNotes::Resizer(UTILRESIZECONTROL *urc)
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

/////////////////////////////////////////////////////////////////////////////////////////
// Launches the incoming note window

void CJabberProto::ProcessIncomingNote(CNoteItem *pNote, bool ok)
{
	if (ok && pNote->IsNotEmpty()) {
		m_notes.insert(pNote);

		XmlNodeIq iq(_T("set"));
		HXML query = iq << XQUERY(JABBER_FEAT_PRIVATE_STORAGE);
		HXML storage = query << XCHILDNS(_T("storage"), JABBER_FEAT_MIRANDA_NOTES);
		m_notes.SaveXml(storage);
		m_ThreadInfo->send(iq);
	}
	else delete pNote;
}

void CJabberProto::ProcessOutgoingNote(CNoteItem *pNote, bool ok)
{
	if (!ok || !pNote->IsNotEmpty()) {
		delete pNote;
		return;
	}

	TCHAR buf[1024];
	mir_sntprintf(buf, SIZEOF(buf), _T("Incoming note: %s\n\n%s\nTags: %s"),
		pNote->GetTitle(), pNote->GetText(), pNote->GetTagsStr());

	JabberCapsBits jcb = GetResourceCapabilites(pNote->GetFrom(), TRUE);

	if (jcb & JABBER_RESOURCE_CAPS_ERROR)
		jcb = JABBER_RESOURCE_CAPS_NONE;

	int nMsgId = SerialNext();

	XmlNode m(_T("message"));
	m << XATTR(_T("type"), _T("chat")) << XATTR(_T("to"), pNote->GetFrom()) << XATTRID(nMsgId);
	m << XCHILD(_T("body"), buf);
	HXML hXmlItem = m << XCHILDNS(_T("x"), JABBER_FEAT_MIRANDA_NOTES) << XCHILD(_T("note"));
	hXmlItem << XATTR(_T("tags"), pNote->GetTagsStr());
	hXmlItem << XCHILD(_T("title"), pNote->GetTitle());
	hXmlItem << XCHILD(_T("text"), pNote->GetText());

	// message receipts XEP priority
	if (jcb & JABBER_CAPS_MESSAGE_RECEIPTS)
		m << XCHILDNS(_T("request"), JABBER_FEAT_MESSAGE_RECEIPTS);
	else if (jcb & JABBER_CAPS_MESSAGE_EVENTS) {
		HXML x = m << XCHILDNS(_T("x"), JABBER_FEAT_MESSAGE_EVENTS);
		x << XCHILD(_T("delivered")); x << XCHILD(_T("offline"));
	}
	else
		nMsgId = -1;

	m_ThreadInfo->send(m);
	delete pNote;
}

bool CJabberProto::OnIncomingNote(const TCHAR *szFrom, HXML hXml)
{
	if (!m_options.AcceptNotes)
		return false;

	if (!szFrom || !hXml) return true;
	CNoteItem *pItem = new CNoteItem(hXml, (TCHAR *)szFrom);
	if (!pItem->IsNotEmpty()) {
		delete pItem;
		return true;
	}

	if (m_options.AutosaveNotes && HContactFromJID(szFrom)) {
		ProcessIncomingNote(pItem, true);
		return false;
	}

	CLISTEVENT cle = { 0 };
	char szService[256];
	mir_snprintf(szService, SIZEOF(szService), "%s%s", m_szModuleName, JS_INCOMING_NOTE_EVENT);
	cle.cbSize = sizeof(CLISTEVENT);
	cle.hIcon = (HICON)LoadIconEx("notes");
	cle.flags = CLEF_PROTOCOLGLOBAL | CLEF_TCHAR;
	cle.hDbEvent = -99;
	cle.lParam = (LPARAM)pItem;
	cle.pszService = szService;
	cle.ptszTooltip = TranslateT("Incoming note");
	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);

	return true;
}

INT_PTR __cdecl CJabberProto::OnIncomingNoteEvent(WPARAM, LPARAM lParam)
{
	CLISTEVENT *pCle = (CLISTEVENT *)lParam;
	CNoteItem *pNote = (CNoteItem *)pCle->lParam;
	if (!pNote)
		return 0;

	CJabberDlgBase *pDlg = new CJabberDlgNoteItem(this, pNote, &CJabberProto::ProcessIncomingNote);
	pDlg->Show();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Menu handling

INT_PTR __cdecl CJabberProto::OnMenuHandleNotes(WPARAM, LPARAM)
{
	UI_SAFE_OPEN_EX(CJabberDlgNotes, m_pDlgNotes, pDlg);
	pDlg->UpdateData();
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuSendNote(WPARAM wParam, LPARAM)
{
	if (wParam) {
		CNoteItem *pItem = new CNoteItem(NULL, ptrT(getTStringA(wParam, "jid")));
		CJabberDlgBase *pDlg = new CJabberDlgNoteItem(this, pItem, &CJabberProto::ProcessOutgoingNote);
		pDlg->Show();
	}

	return 0;
}
