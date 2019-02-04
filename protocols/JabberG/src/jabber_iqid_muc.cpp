/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-19 Miranda NG team

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

void CJabberProto::SetMucConfig(HXML node, void *from)
{
	if (m_ThreadInfo && from) {
		XmlNodeIq iq(L"set", SerialNext(), (wchar_t*)from);
		HXML query = iq << XQUERY(JABBER_FEAT_MUC_OWNER);
		XmlAddChild(query, node);
		m_ThreadInfo->send(iq);
	}
}

// RECVED: room config form
// ACTION: show the form
void CJabberProto::OnIqResultGetMuc(HXML iqNode, CJabberIqInfo*)
{
	debugLogA("<iq/> iqIdGetMuc");
	const wchar_t *type = XmlGetAttrValue(iqNode, L"type");
	if (type == nullptr)
		return;
	const wchar_t *from = XmlGetAttrValue(iqNode, L"from");
	if (from == nullptr)
		return;

	if (!mir_wstrcmp(type, L"result")) {
		HXML queryNode = XmlGetChild(iqNode, L"query");
		if (queryNode != nullptr) {
			const wchar_t *str = XmlGetAttrValue(queryNode, L"xmlns");
			if (!mir_wstrcmp(str, JABBER_FEAT_MUC_OWNER)) {
				HXML xNode = XmlGetChild(queryNode, L"x");
				if (xNode != nullptr) {
					str = XmlGetAttrValue(xNode, L"xmlns");
					if (!mir_wstrcmp(str, JABBER_FEAT_DATA_FORMS))
						//LaunchForm(xNode);
						FormCreateDialog(xNode, L"Jabber Conference Room Configuration", &CJabberProto::SetMucConfig, mir_wstrdup(from));
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	int idc;
	char *title;
	char *icon;
	bool push;
}
static buttons[] =
{
	{ IDC_BTN_FILTERAPPLY, "Apply filter", "sd_filter_apply", false },
	{ IDC_BTN_FILTERRESET, "Reset filter", "sd_filter_reset", false },
};

class CJabberMucJidListDlg : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	void FreeList()
	{
		// Free lParam of the displayed list items
		int count = m_list.GetItemCount();

		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;
		for (int i = 0; i < count; i++) {
			lvi.iItem = i;
			if (m_list.GetItem(&lvi) == TRUE)
				if (lvi.lParam != -1 && lvi.lParam != 0)
					mir_free((void *)lvi.lParam);
		}
		m_list.DeleteAllItems();
	}

	void FillJidList()
	{
		wchar_t *filter = nullptr;
		if (GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FILTER), GWLP_USERDATA)) {
			int filterLength = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_FILTER)) + 1;
			filter = (wchar_t *)_alloca(filterLength * sizeof(wchar_t));
			GetDlgItemText(m_hwnd, IDC_FILTER, filter, filterLength);
		}

		if (!m_info)
			return;

		m_list.SendMsg(WM_SETREDRAW, FALSE, 0);

		FreeList();

		// Populate displayed list from iqNode
		LVITEM lvi = {};
		wchar_t tszItemText[JABBER_MAX_JID_LEN + 256];
		HXML iqNode = m_info->iqNode;
		if (iqNode != nullptr) {
			const wchar_t *from = XmlGetAttrValue(iqNode, L"from");
			if (from != nullptr) {
				HXML queryNode = XmlGetChild(iqNode, L"query");
				if (queryNode != nullptr) {
					lvi.iItem = 0;
					for (int i = 0;; i++) {
						HXML itemNode = XmlGetChild(queryNode, i);
						if (!itemNode)
							break;

						const wchar_t *jid = XmlGetAttrValue(itemNode, L"jid");
						if (jid == nullptr)
							continue;

						lvi.pszText = (wchar_t*)jid;
						if (m_info->type == MUC_BANLIST) {
							const wchar_t *reason = XmlGetText(XmlGetChild(itemNode, L"reason"));
							if (reason != nullptr) {
								mir_snwprintf(tszItemText, L"%s (%s)", jid, reason);
								lvi.pszText = tszItemText;
							}
						}
						else if (m_info->type == MUC_VOICELIST || m_info->type == MUC_MODERATORLIST) {
							const wchar_t *nick = XmlGetAttrValue(itemNode, L"nick");
							if (nick != nullptr) {
								mir_snwprintf(tszItemText, L"%s (%s)", nick, jid);
								lvi.pszText = tszItemText;
							}
						}

						if (filter && *filter && !JabberStrIStr(lvi.pszText, filter))
							continue;

						lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
						lvi.iSubItem = 0;
						lvi.lParam = (LPARAM)mir_wstrdup(jid);
						m_list.InsertItem(&lvi);
						lvi.iItem++;
					}
				}
			}
		}

		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM | LVIF_IMAGE;
		lvi.lParam = -1;
		lvi.iImage = 1;
		m_list.InsertItem(&lvi);

		m_list.SendMsg(WM_SETREDRAW, TRUE, 0);
		RedrawWindow(m_list.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
	}

	JABBER_MUC_JIDLIST_INFO *m_info;

	CCtrlButton btnReset, btnApply;
	CCtrlListView m_list;

public:
	CJabberMucJidListDlg(CJabberProto *ppro, JABBER_MUC_JIDLIST_INFO *pInfo) :
		CJabberDlgBase(ppro, IDD_JIDLIST),
		m_info(pInfo),
		m_list(this, IDC_LIST),
		btnApply(this, IDC_BTN_FILTERAPPLY),
		btnReset(this, IDC_BTN_FILTERRESET)
	{
		m_list.OnClick = Callback(this, &CJabberMucJidListDlg::list_OnClick);

		btnApply.OnClick = Callback(this, &CJabberMucJidListDlg::onClick_Apply);
		btnReset.OnClick = Callback(this, &CJabberMucJidListDlg::onClick_Reset);
	}

	bool OnInitDialog() override
	{
		HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 0);
		ImageList_AddIcon(hImageList, g_LoadIconEx("delete"));
		ImageList_AddIcon(hImageList, g_LoadIconEx("addcontact"));
		m_list.SetImageList(hImageList, LVSIL_SMALL);

		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);

		RECT rc;
		GetClientRect(m_list.GetHwnd(), &rc);

		LVCOLUMN lvc = {};
		lvc.mask = LVCF_WIDTH;
		lvc.cx = rc.right;
		m_list.InsertColumn(0, &lvc);

		Refresh(m_info);

		for (auto &it : buttons) {
			SendDlgItemMessage(m_hwnd, it.idc, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_proto->LoadIconEx(it.icon));
			SendDlgItemMessage(m_hwnd, it.idc, BUTTONSETASFLATBTN, TRUE, 0);
			SendDlgItemMessage(m_hwnd, it.idc, BUTTONADDTOOLTIP, (WPARAM)it.title, 0);
			if (it.push)
				SendDlgItemMessage(m_hwnd, it.idc, BUTTONSETASPUSHBTN, TRUE, 0);
		}

		Utils_RestoreWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "jidListWnd_");
		return true;
	}

	bool OnClose() override
	{
		FreeList();
		m_proto->GetMucDlg(m_info->type) = nullptr;
		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "jidListWnd_");
	}

	INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		INT_PTR ret = CSuper::DlgProc(uMsg, wParam, lParam);
		if (uMsg == WM_SIZE) {
			RECT rc;
			GetClientRect(m_list.GetHeader(), &rc);
			m_list.SetColumnWidth(0, rc.right - rc.left);
		}

		return ret;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_LIST:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

		case IDC_FILTER:
			return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM | RD_ANCHORX_WIDTH;

		case IDC_BTN_FILTERRESET:
		case IDC_BTN_FILTERAPPLY:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	void Refresh(JABBER_MUC_JIDLIST_INFO *pInfo)
	{
		// Set new GWL_USERDATA
		m_info = pInfo;

		// Populate displayed list from iqNode
		wchar_t title[256];
		mir_wstrncpy(title, TranslateT("JID List"), _countof(title));
		if (pInfo != nullptr) {
			HXML iqNode = pInfo->iqNode;
			if (iqNode != nullptr) {
				const wchar_t *from = XmlGetAttrValue(iqNode, L"from");
				if (from != nullptr) {
					pInfo->roomJid = mir_wstrdup(from);
					HXML queryNode = XmlGetChild(iqNode, L"query");
					if (queryNode != nullptr)
						mir_snwprintf(title, TranslateT("%s, %d items (%s)"), pInfo->type2str(), XmlGetChildCount(queryNode), from);
				}
			}
		}
		SetWindowText(m_hwnd, title);

		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FILTER), GWLP_USERDATA, 0);
		FillJidList();
	}

	void list_OnClick(CCtrlListView::TEventInfo*)
	{
		LVHITTESTINFO hti;
		hti.pt.x = (short)LOWORD(GetMessagePos());
		hti.pt.y = (short)HIWORD(GetMessagePos());
		ScreenToClient(m_list.GetHwnd(), &hti.pt);
		if (m_list.SubItemHitTest(&hti) == -1)
			return;

		if (!(hti.flags & LVHT_ONITEMICON))
			return;

		wchar_t text[128];
		LVITEM lvi;
		lvi.mask = LVIF_PARAM | LVIF_TEXT;
		lvi.iItem = hti.iItem;
		lvi.iSubItem = 0;
		lvi.pszText = text;
		lvi.cchTextMax = _countof(text);
		m_list.GetItem(&lvi);
		if (lvi.lParam == -1) {
			CMStringW szBuffer(m_info->type2str());
			if (!m_proto->EnterString(szBuffer, nullptr, ESF_COMBO, "gcAddNick_"))
				return;

			// Trim leading and trailing whitespaces
			szBuffer.Trim();
			if (szBuffer.IsEmpty())
				return;

			CMStringW rsn(m_info->type2str());
			if (m_info->type == MUC_BANLIST) {
				m_proto->EnterString(rsn, TranslateT("Reason to ban"), ESF_COMBO, "gcAddReason_");
				if (szBuffer)
					m_proto->AddMucListItem(m_info, szBuffer, rsn);
				else
					m_proto->AddMucListItem(m_info, szBuffer);
			}
			else m_proto->AddMucListItem(m_info, szBuffer);
		}
		else { // delete
			wchar_t msgText[128];
			mir_snwprintf(msgText, TranslateT("Removing %s?"), text);
			if (MessageBox(m_hwnd, msgText, m_info->type2str(), MB_YESNO | MB_SETFOREGROUND) == IDYES) {
				m_proto->DeleteMucListItem(m_info, (wchar_t*)lvi.lParam);
				mir_free((void *)lvi.lParam);
				m_list.DeleteItem(hti.iItem);
			}
		}
	}

	void onClick_Apply(CCtrlButton*)
	{
		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FILTER), GWLP_USERDATA, 1);
		FillJidList();
	}

	void onClick_Reset(CCtrlButton*)
	{
		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FILTER), GWLP_USERDATA, 0);
		FillJidList();
	}
};

static void CALLBACK JabberMucJidListCreateDialogApcProc(void* param)
{
	JABBER_MUC_JIDLIST_INFO *jidListInfo = (JABBER_MUC_JIDLIST_INFO *)param;
	if (jidListInfo == nullptr)
		return;

	HXML iqNode = jidListInfo->iqNode;
	if (iqNode == nullptr)
		return;

	const wchar_t *from = XmlGetAttrValue(iqNode, L"from");
	if (from == nullptr)
		return;

	HXML queryNode = XmlGetChild(iqNode, L"query");
	if (queryNode == nullptr)
		return;

	CJabberProto *ppro = jidListInfo->ppro;
	CJabberMucJidListDlg *&pHwndJidList = ppro->GetMucDlg(jidListInfo->type);

	if (pHwndJidList != nullptr) {
		SetForegroundWindow(pHwndJidList->GetHwnd());
		pHwndJidList->Refresh(jidListInfo);
	}
	else {
		pHwndJidList = new CJabberMucJidListDlg(ppro, jidListInfo);
		pHwndJidList->Show();
	}
}

void CJabberProto::OnIqResultMucGetJidList(HXML iqNode, JABBER_MUC_JIDLIST_TYPE listType)
{
	const wchar_t *type = XmlGetAttrValue(iqNode, L"type");
	if (type == nullptr)
		return;

	if (!mir_wstrcmp(type, L"result")) {
		JABBER_MUC_JIDLIST_INFO *jidListInfo = new JABBER_MUC_JIDLIST_INFO;
		if (jidListInfo != nullptr) {
			jidListInfo->type = listType;
			jidListInfo->ppro = this;
			jidListInfo->roomJid = nullptr;	// Set in the dialog procedure
			if ((jidListInfo->iqNode = xmlCopyNode(iqNode)) != nullptr)
				CallFunctionAsync(JabberMucJidListCreateDialogApcProc, jidListInfo);
			else
				mir_free(jidListInfo);
		}
	}
}

void CJabberProto::MucShutdown()
{
	UI_SAFE_CLOSE(m_pDlgMucVoiceList);
	UI_SAFE_CLOSE(m_pDlgMucMemberList);
	UI_SAFE_CLOSE(m_pDlgMucModeratorList);
	UI_SAFE_CLOSE(m_pDlgMucBanList);
	UI_SAFE_CLOSE(m_pDlgMucAdminList);
	UI_SAFE_CLOSE(m_pDlgMucOwnerList);
}

CJabberMucJidListDlg*& CJabberProto::GetMucDlg(JABBER_MUC_JIDLIST_TYPE type)
{
	switch (type) {
	case MUC_VOICELIST:
		return m_pDlgMucVoiceList;
	case MUC_MEMBERLIST:
		return m_pDlgMucMemberList;
	case MUC_MODERATORLIST:
		return m_pDlgMucModeratorList;
	case MUC_BANLIST:
		return m_pDlgMucBanList;
	case MUC_ADMINLIST:
		return m_pDlgMucAdminList;
	case MUC_OWNERLIST:
		return m_pDlgMucOwnerList;
	}

	// never happens. just to make compiler happy
	static CJabberMucJidListDlg *pStub = nullptr;
	return pStub;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::OnIqResultMucGetVoiceList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetVoiceList");
	OnIqResultMucGetJidList(iqNode, MUC_VOICELIST);
}

void CJabberProto::OnIqResultMucGetMemberList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetMemberList");
	OnIqResultMucGetJidList(iqNode, MUC_MEMBERLIST);
}

void CJabberProto::OnIqResultMucGetModeratorList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetModeratorList");
	OnIqResultMucGetJidList(iqNode, MUC_MODERATORLIST);
}

void CJabberProto::OnIqResultMucGetBanList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetBanList");
	OnIqResultMucGetJidList(iqNode, MUC_BANLIST);
}

void CJabberProto::OnIqResultMucGetAdminList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetAdminList");
	OnIqResultMucGetJidList(iqNode, MUC_ADMINLIST);
}

void CJabberProto::OnIqResultMucGetOwnerList(HXML iqNode, CJabberIqInfo *)
{
	debugLogA("<iq/> iqResultMucGetOwnerList");
	OnIqResultMucGetJidList(iqNode, MUC_OWNERLIST);
}

/////////////////////////////////////////////////////////////////////////////////////////

JABBER_MUC_JIDLIST_INFO::~JABBER_MUC_JIDLIST_INFO()
{
	xmlDestroyNode(iqNode);
	mir_free(roomJid);
}

wchar_t* JABBER_MUC_JIDLIST_INFO::type2str() const
{
	switch (type) {
		case MUC_VOICELIST:     return TranslateT("Voice List");
		case MUC_MEMBERLIST:    return TranslateT("Member List");
		case MUC_MODERATORLIST: return TranslateT("Moderator List");
		case MUC_BANLIST:       return TranslateT("Ban List");
		case MUC_ADMINLIST:     return TranslateT("Admin List");
		case MUC_OWNERLIST:     return TranslateT("Owner List");
		default:                return TranslateT("JID List");
	}
}
